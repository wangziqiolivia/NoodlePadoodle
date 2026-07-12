#include <Arduino.h>


// ==========================================================
// CONFIG
// ==========================================================

#define NUM_STRINGS 17

// 两组 HX711 时钟
#define CLK_GROUP_A 4
#define CLK_GROUP_B 2

#define SERIAL_BAUD 115200

// 等待正常通道准备好的最长时间
#define HX711_TIMEOUT_MS 1500

// 正常通道连续超时三次才复位
#define MAX_CONSECUTIVE_TIMEOUTS 3

// String 13 或 17 无效时输出这个数字
// 它超出 HX711 的正常 24-bit 范围
const int32_t INVALID_READING = -99999999;


// ==========================================================
// DOUT PINS
// ==========================================================

const uint8_t DOUT_PINS[NUM_STRINGS] = {
    13,  // String 1
    14,  // String 2
    15,  // String 3
    16,  // String 4
    17,  // String 5
    18,  // String 6
    19,  // String 7
    21,  // String 8
    22,  // String 9

    23,  // String 10
    25,  // String 11
    26,  // String 12
    27,  // String 13：可选、不阻塞
    32,  // String 14
    33,  // String 15
    34,  // String 16
    36   // String 17：可选、不阻塞
};


// ==========================================================
// CLOCK GROUPS
// ==========================================================

// String 1～9，SCK 接 GPIO4
const uint8_t GROUP_A[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8
};

const uint8_t GROUP_A_COUNT =
    sizeof(GROUP_A) / sizeof(GROUP_A[0]);


// String 10～17，SCK 接 GPIO2
const uint8_t GROUP_B[] = {
    9, 10, 11, 12, 13, 14, 15, 16
};

const uint8_t GROUP_B_COUNT =
    sizeof(GROUP_B) / sizeof(GROUP_B[0]);


// ==========================================================
// STATE
// ==========================================================

int32_t forceValues[NUM_STRINGS];

int consecutiveTimeouts = 0;


// ==========================================================
// OPTIONAL CHANNELS
// ==========================================================

bool isOptionalChannel(uint8_t stringIndex) {

    // 数组下标 12 = String 13
    // 数组下标 16 = String 17
    return (
        stringIndex == 12 ||
        stringIndex == 16
    );
}


// ==========================================================
// READY CHECK
// ==========================================================

bool isChannelReady(uint8_t stringIndex) {

    return (
        digitalRead(
            DOUT_PINS[stringIndex]
        ) == LOW
    );
}


/*
   只检查必须工作的 15 路。

   String 13 和 String 17 即使没有准备好，
   也不会阻挡整个系统。
*/
bool allRequiredChannelsReady() {

    for (uint8_t i = 0; i < NUM_STRINGS; i++) {

        if (isOptionalChannel(i)) {
            continue;
        }

        if (!isChannelReady(i)) {
            return false;
        }
    }

    return true;
}


// ==========================================================
// WAIT FOR REQUIRED CHANNELS
// ==========================================================

bool waitForRequiredChannels(
    uint32_t timeoutMs
) {

    uint32_t startTime = millis();

    // 等待转换期间，两个时钟都必须为 LOW
    digitalWrite(CLK_GROUP_A, LOW);
    digitalWrite(CLK_GROUP_B, LOW);

    while (
        millis() - startTime < timeoutMs
    ) {

        if (allRequiredChannelsReady()) {

            // 再确认一次，减少瞬间干扰
            delayMicroseconds(10);

            if (allRequiredChannelsReady()) {
                return true;
            }
        }

        delay(1);
    }

    return false;
}


// ==========================================================
// RESET HX711
// ==========================================================

void resetClockGroup(uint8_t clockPin) {

    digitalWrite(clockPin, LOW);
    delayMicroseconds(10);

    // 高电平超过 60 微秒，进入 power-down
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(100);

    // 拉低以后退出 power-down
    digitalWrite(clockPin, LOW);
}


void resetAllHX711() {

    resetClockGroup(CLK_GROUP_A);
    resetClockGroup(CLK_GROUP_B);

    // 10 SPS 下等待第一轮新转换
    delay(500);
}


// ==========================================================
// PRINT REQUIRED CHANNEL ERRORS
// ==========================================================

void printNotReadyRequiredChannels() {

    Serial.print("# REQUIRED NOT READY: ");

    bool found = false;

    for (uint8_t i = 0; i < NUM_STRINGS; i++) {

        if (isOptionalChannel(i)) {
            continue;
        }

        if (!isChannelReady(i)) {

            found = true;

            Serial.print("String ");
            Serial.print(i + 1);

            Serial.print(" GPIO");
            Serial.print(DOUT_PINS[i]);

            Serial.print(" | ");
        }
    }

    if (!found) {
        Serial.print("none");
    }

    Serial.println();
}


// ==========================================================
// INVALID RAW PATTERN CHECK
// ==========================================================

bool isInvalidRawPattern(int32_t value) {

    /*
       这些数字是之前出现过的明显异常位图：

       0        = 0x000000
       -1       = 0xFFFFFF
       8388607  = 0x7FFFFF
       4194303  = 0x3FFFFF
       2097151  = 0x1FFFFF
       1048575  = 0x0FFFFF
       524287   = 0x07FFFF
    */

    return (
        value == 0 ||
        value == -1 ||
        value == 8388607 ||
        value == -8388608 ||
        value == 4194303 ||
        value == 2097151 ||
        value == 1048575 ||
        value == 524287
    );
}


// ==========================================================
// READ ONE CLOCK GROUP
// ==========================================================

void readHX711Group(
    uint8_t clockPin,
    const uint8_t group[],
    uint8_t groupCount,
    int32_t outputValues[]
) {

    uint32_t rawValues[NUM_STRINGS];

    bool readyAtStart[NUM_STRINGS];


    // ------------------------------------------------------
    // INITIALIZE THIS GROUP
    // ------------------------------------------------------

    for (
        uint8_t groupPosition = 0;
        groupPosition < groupCount;
        groupPosition++
    ) {

        uint8_t stringIndex =
            group[groupPosition];

        rawValues[stringIndex] = 0;

        /*
           在发送时钟前记住该通道是否准备好。

           对 String 13 和 17：
           如果现在没有准备好，之后直接标记无效。
        */

        readyAtStart[stringIndex] =
            isChannelReady(stringIndex);
    }


    // ------------------------------------------------------
    // READ 24 BITS
    // ------------------------------------------------------

    noInterrupts();

    for (
        uint8_t bitIndex = 0;
        bitIndex < 24;
        bitIndex++
    ) {

        // HX711 在时钟边沿移出下一位
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(1);

        digitalWrite(clockPin, LOW);
        delayMicroseconds(1);


        // 在 CLK 为 LOW 时读取本组全部 DOUT
        for (
            uint8_t groupPosition = 0;
            groupPosition < groupCount;
            groupPosition++
        ) {

            uint8_t stringIndex =
                group[groupPosition];

            rawValues[stringIndex] <<= 1;

            if (
                digitalRead(
                    DOUT_PINS[stringIndex]
                ) == HIGH
            ) {

                rawValues[stringIndex] |= 1UL;
            }
        }
    }


    // ------------------------------------------------------
    // 25TH PULSE
    //
    // 下一轮：
    // Channel A
    // Gain 128
    // ------------------------------------------------------

    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);

    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);

    interrupts();


    // ------------------------------------------------------
    // CONVERT VALUES
    // ------------------------------------------------------

    for (
        uint8_t groupPosition = 0;
        groupPosition < groupCount;
        groupPosition++
    ) {

        uint8_t stringIndex =
            group[groupPosition];


        /*
           String 13 和 17 如果读之前没有准备好，
           不使用这次位流。
        */

        if (
            isOptionalChannel(stringIndex) &&
            !readyAtStart[stringIndex]
        ) {

            outputValues[stringIndex] =
                INVALID_READING;

            continue;
        }


        // 24-bit 符号扩展到 32-bit
        if (
            rawValues[stringIndex]
            & 0x800000UL
        ) {

            rawValues[stringIndex] |=
                0xFF000000UL;
        }

        int32_t signedValue =
            static_cast<int32_t>(
                rawValues[stringIndex]
            );


        /*
           String 13 和 17 即使 DOUT 为 LOW，
           如果仍读到明显的错误位图，
           也将其标记为无效。
        */

        if (
            isOptionalChannel(stringIndex) &&
            isInvalidRawPattern(signedValue)
        ) {

            outputValues[stringIndex] =
                INVALID_READING;

            continue;
        }


        outputValues[stringIndex] =
            signedValue;
    }
}


// ==========================================================
// PRINT OPTIONAL CHANNEL STATUS
// ==========================================================

void printOptionalStatus(
    const int32_t values[]
) {

    bool string13Invalid =
        values[12] == INVALID_READING;

    bool string17Invalid =
        values[16] == INVALID_READING;


    if (
        !string13Invalid &&
        !string17Invalid
    ) {
        return;
    }


    Serial.print("# OPTIONAL INVALID: ");

    if (string13Invalid) {
        Serial.print("String 13 | ");
    }

    if (string17Invalid) {
        Serial.print("String 17 | ");
    }

    Serial.println();
}


// ==========================================================
// SEND CSV
// ==========================================================

void sendValuesCSV(
    const int32_t values[]
) {

    /*
       始终输出 17 个数字。

       如果 String 13 或 17 无效：

       对应位置输出 -99999999。
    */

    for (uint8_t i = 0; i < NUM_STRINGS; i++) {

        Serial.print(values[i]);

        if (i < NUM_STRINGS - 1) {
            Serial.print(',');
        }
    }

    Serial.println();
}


// ==========================================================
// SETUP
// ==========================================================

void setup() {

    Serial.begin(SERIAL_BAUD);


    // ------------------------------------------------------
    // CLOCK OUTPUTS
    // ------------------------------------------------------

    pinMode(CLK_GROUP_A, OUTPUT);
    pinMode(CLK_GROUP_B, OUTPUT);

    digitalWrite(CLK_GROUP_A, LOW);
    digitalWrite(CLK_GROUP_B, LOW);


    // ------------------------------------------------------
    // DOUT INPUTS
    // ------------------------------------------------------

    for (uint8_t i = 0; i < NUM_STRINGS; i++) {

        pinMode(
            DOUT_PINS[i],
            INPUT
        );
    }


    delay(500);


    Serial.println();
    Serial.println(
        "================================="
    );
    Serial.println(
        " NoodlePadoodle 17 HX711 Reader"
    );
    Serial.println(
        " String 13 and 17 are optional"
    );
    Serial.println(
        "================================="
    );

    Serial.println(
        "Group A: Strings 1-9, CLK GPIO4"
    );

    Serial.println(
        "Group B: Strings 10-17, CLK GPIO2"
    );

    Serial.println(
        "Invalid optional value: -99999999"
    );

    Serial.println(
        "Resetting HX711 groups..."
    );


    resetAllHX711();


    Serial.println("READY");
}


// ==========================================================
// MAIN LOOP
// ==========================================================

void loop() {

    digitalWrite(CLK_GROUP_A, LOW);
    digitalWrite(CLK_GROUP_B, LOW);


    // ======================================================
    // WAIT ONLY FOR THE 15 REQUIRED CHANNELS
    // ======================================================

    if (
        !waitForRequiredChannels(
            HX711_TIMEOUT_MS
        )
    ) {

        consecutiveTimeouts++;

        Serial.print("# TIMEOUT ");
        Serial.print(consecutiveTimeouts);
        Serial.print("/");
        Serial.print(
            MAX_CONSECUTIVE_TIMEOUTS
        );
        Serial.print(" ");

        printNotReadyRequiredChannels();


        /*
           String 13 和 String 17 不会触发这里。

           只有其他 15 路连续失败，
           才会重置全部模块。
        */

        if (
            consecutiveTimeouts
            >= MAX_CONSECUTIVE_TIMEOUTS
        ) {

            Serial.println(
                "# Resetting HX711 groups..."
            );

            resetAllHX711();

            consecutiveTimeouts = 0;
        }

        return;
    }


    // ======================================================
    // READ GROUP A
    // ======================================================

    readHX711Group(
        CLK_GROUP_A,
        GROUP_A,
        GROUP_A_COUNT,
        forceValues
    );


    // ======================================================
    // READ GROUP B
    // ======================================================

    readHX711Group(
        CLK_GROUP_B,
        GROUP_B,
        GROUP_B_COUNT,
        forceValues
    );


    consecutiveTimeouts = 0;


    // ======================================================
    // OUTPUT
    // ======================================================

    // 纯 CSV：始终包含 17 个位置
    sendValuesCSV(forceValues);

    // 额外诊断行以 # 开头
    printOptionalStatus(forceValues);
}