#include <Arduino.h>


// ==========================================================
// CONFIG
// ==========================================================

#define SERIAL_BAUD 115200

// 两组 HX711 公共时钟
#define CLK_GROUP_A 4
#define CLK_GROUP_B 2

// HX711 时钟脉冲宽度
#define CLOCK_PULSE_US 8

// 最长等待时间
#define HX711_TIMEOUT_MS 1500

// 连续失败三次才复位对应组
#define MAX_CONSECUTIVE_TIMEOUTS 3


// ==========================================================
// CHANNEL STRUCTURE
// ==========================================================

struct HX711Channel {
    uint8_t stringNumber;
    uint8_t doutPin;
};


// ==========================================================
// STABLE CHANNELS
// ==========================================================

// Group A：SCK 接 GPIO4
// 使用 String 5、7、9
const HX711Channel GROUP_A[] = {
    {5, 17},
    {7, 19},
    {9, 22}
};

const uint8_t GROUP_A_COUNT =
    sizeof(GROUP_A) / sizeof(GROUP_A[0]);


// Group B：SCK 接 GPIO2
// 使用 String 11、12、14、15、16
const HX711Channel GROUP_B[] = {
    {11, 25},
    {12, 26},
    {14, 32},
    {15, 33},
    {16, 34}
};

const uint8_t GROUP_B_COUNT =
    sizeof(GROUP_B) / sizeof(GROUP_B[0]);


// ==========================================================
// DATA STORAGE
// ==========================================================

int32_t groupAValues[GROUP_A_COUNT];
int32_t groupBValues[GROUP_B_COUNT];

int groupATimeouts = 0;
int groupBTimeouts = 0;


// ==========================================================
// RESET ONE GROUP
// ==========================================================

void resetGroup(uint8_t clockPin) {

    digitalWrite(clockPin, LOW);
    delayMicroseconds(10);

    // HIGH 超过 60 us，使 HX711 进入 power-down
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(100);

    // 拉回 LOW，退出 power-down 并开始新转换
    digitalWrite(clockPin, LOW);

    // 10 SPS 模式下等待第一轮数据完成
    delay(500);
}


// ==========================================================
// CHECK GROUP READY
// ==========================================================

bool isGroupReady(
    const HX711Channel channels[],
    uint8_t channelCount
) {

    for (uint8_t i = 0; i < channelCount; i++) {

        // DOUT LOW 表示数据准备完成
        if (digitalRead(channels[i].doutPin) == HIGH) {
            return false;
        }
    }

    return true;
}


// ==========================================================
// WAIT FOR GROUP
// ==========================================================

bool waitForGroup(
    uint8_t clockPin,
    const HX711Channel channels[],
    uint8_t channelCount,
    uint32_t timeoutMs
) {

    uint32_t startTime = millis();

    // 等待期间 SCK 必须保持 LOW
    digitalWrite(clockPin, LOW);

    while (millis() - startTime < timeoutMs) {

        if (isGroupReady(channels, channelCount)) {

            // 再确认一次，避免瞬间干扰
            delayMicroseconds(10);

            if (isGroupReady(channels, channelCount)) {
                return true;
            }
        }

        delay(1);
    }

    return false;
}


// ==========================================================
// PRINT NOT-READY CHANNELS
// ==========================================================

void printNotReadyChannels(
    const HX711Channel channels[],
    uint8_t channelCount
) {

    bool found = false;

    for (uint8_t i = 0; i < channelCount; i++) {

        if (digitalRead(channels[i].doutPin) == HIGH) {

            found = true;

            Serial.print("String ");
            Serial.print(channels[i].stringNumber);

            Serial.print(" GPIO");
            Serial.print(channels[i].doutPin);

            Serial.print(" | ");
        }
    }

    if (!found) {
        Serial.print("none");
    }
}


// ==========================================================
// READ ONE GROUP
// ==========================================================

void readGroup(
    uint8_t clockPin,
    const HX711Channel channels[],
    uint8_t channelCount,
    int32_t outputValues[]
) {

    // 最大组只有 5 路，使用固定数组避免变长数组问题
    uint32_t rawValues[8];

    for (uint8_t i = 0; i < channelCount; i++) {
        rawValues[i] = 0;
    }

    /*
       关闭中断，防止 CLK 高电平被延长。
    */
    noInterrupts();


    // ======================================================
    // READ 24 BITS
    // ======================================================

    for (
        uint8_t bitIndex = 0;
        bitIndex < 24;
        bitIndex++
    ) {

        // 产生时钟上升沿
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(CLOCK_PULSE_US);

        // 拉低后读取数据
        digitalWrite(clockPin, LOW);
        delayMicroseconds(CLOCK_PULSE_US);

        for (uint8_t i = 0; i < channelCount; i++) {

            rawValues[i] <<= 1;

            if (digitalRead(channels[i].doutPin) == HIGH) {
                rawValues[i] |= 1UL;
            }
        }
    }


    // ======================================================
    // 25TH CLOCK
    //
    // 下一次读取 Channel A，Gain 128
    // ======================================================

    digitalWrite(clockPin, HIGH);
    delayMicroseconds(CLOCK_PULSE_US);

    digitalWrite(clockPin, LOW);
    delayMicroseconds(CLOCK_PULSE_US);


    interrupts();


    // ======================================================
    // SIGN EXTENSION
    // ======================================================

    for (uint8_t i = 0; i < channelCount; i++) {

        // 24-bit signed → 32-bit signed
        if (rawValues[i] & 0x800000UL) {
            rawValues[i] |= 0xFF000000UL;
        }

        outputValues[i] =
            static_cast<int32_t>(rawValues[i]);
    }
}


// ==========================================================
// SEND CSV
// ==========================================================

void sendCSV() {

    /*
       固定输出顺序：

       S5,S7,S9,S11,S12,S14,S15,S16
    */

    for (uint8_t i = 0; i < GROUP_A_COUNT; i++) {

        Serial.print(groupAValues[i]);
        Serial.print(',');
    }

    for (uint8_t i = 0; i < GROUP_B_COUNT; i++) {

        Serial.print(groupBValues[i]);

        if (i < GROUP_B_COUNT - 1) {
            Serial.print(',');
        }
    }

    Serial.println();
}


// ==========================================================
// GROUP A TIMEOUT
// ==========================================================

void handleGroupATimeout() {

    groupATimeouts++;

    Serial.print("# GROUP A TIMEOUT ");
    Serial.print(groupATimeouts);
    Serial.print("/");
    Serial.print(MAX_CONSECUTIVE_TIMEOUTS);
    Serial.print(" NOT READY: ");

    printNotReadyChannels(
        GROUP_A,
        GROUP_A_COUNT
    );

    Serial.println();

    if (groupATimeouts >= MAX_CONSECUTIVE_TIMEOUTS) {

        Serial.println("# Resetting Group A");

        resetGroup(CLK_GROUP_A);

        groupATimeouts = 0;
    }
}


// ==========================================================
// GROUP B TIMEOUT
// ==========================================================

void handleGroupBTimeout() {

    groupBTimeouts++;

    Serial.print("# GROUP B TIMEOUT ");
    Serial.print(groupBTimeouts);
    Serial.print("/");
    Serial.print(MAX_CONSECUTIVE_TIMEOUTS);
    Serial.print(" NOT READY: ");

    printNotReadyChannels(
        GROUP_B,
        GROUP_B_COUNT
    );

    Serial.println();

    if (groupBTimeouts >= MAX_CONSECUTIVE_TIMEOUTS) {

        Serial.println("# Resetting Group B");

        resetGroup(CLK_GROUP_B);

        groupBTimeouts = 0;
    }
}


// ==========================================================
// SETUP
// ==========================================================

void setup() {

    Serial.begin(SERIAL_BAUD);


    // ------------------------------------------------------
    // CLOCK PINS
    // ------------------------------------------------------

    pinMode(CLK_GROUP_A, OUTPUT);
    pinMode(CLK_GROUP_B, OUTPUT);

    digitalWrite(CLK_GROUP_A, LOW);
    digitalWrite(CLK_GROUP_B, LOW);


    // ------------------------------------------------------
    // GROUP A DOUT PINS
    // ------------------------------------------------------

    for (uint8_t i = 0; i < GROUP_A_COUNT; i++) {
        pinMode(GROUP_A[i].doutPin, INPUT);
    }


    // ------------------------------------------------------
    // GROUP B DOUT PINS
    // ------------------------------------------------------

    for (uint8_t i = 0; i < GROUP_B_COUNT; i++) {
        pinMode(GROUP_B[i].doutPin, INPUT);
    }


    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" NoodlePadoodle 8 Stable Strings");
    Serial.println("======================================");

    Serial.println("Using strings:");
    Serial.println("5, 7, 9, 11, 12, 14, 15, 16");

    Serial.println();
    Serial.println("CSV order:");
    Serial.println("S5,S7,S9,S11,S12,S14,S15,S16");

    Serial.println();
    Serial.println("Resetting Group A...");
    resetGroup(CLK_GROUP_A);

    Serial.println("Resetting Group B...");
    resetGroup(CLK_GROUP_B);

    Serial.println("READY");
}


// ==========================================================
// MAIN LOOP
// ==========================================================

void loop() {

    digitalWrite(CLK_GROUP_A, LOW);
    digitalWrite(CLK_GROUP_B, LOW);


    // ======================================================
    // WAIT FOR GROUP A
    // ======================================================

    if (!waitForGroup(
        CLK_GROUP_A,
        GROUP_A,
        GROUP_A_COUNT,
        HX711_TIMEOUT_MS
    )) {

        handleGroupATimeout();
        return;
    }


    // ======================================================
    // WAIT FOR GROUP B
    // ======================================================

    if (!waitForGroup(
        CLK_GROUP_B,
        GROUP_B,
        GROUP_B_COUNT,
        HX711_TIMEOUT_MS
    )) {

        handleGroupBTimeout();
        return;
    }


    groupATimeouts = 0;
    groupBTimeouts = 0;


    // ======================================================
    // READ BOTH GROUPS
    // ======================================================

    readGroup(
        CLK_GROUP_A,
        GROUP_A,
        GROUP_A_COUNT,
        groupAValues
    );

    readGroup(
        CLK_GROUP_B,
        GROUP_B,
        GROUP_B_COUNT,
        groupBValues
    );


    // ======================================================
    // OUTPUT
    // ======================================================

    sendCSV();
}