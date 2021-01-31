#ifndef I2C_EXTENSION_H
#define I2C_EXTENSION_H

// /Users/**/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/libraries/Wire/src/
#include <Wire.h>

#define I2C_TIMEOUT 2000    // Wait until the receiving process is completed
#define I2C_PARAM_SIZE 16    // 2[byte] x 8
#define I2C_REQUEST_SIZE 11 // slave -> master
enum I2C_COM_ID {I2C_ID_DOOR, I2C_ID_NEAR, I2C_ID_MIDDLE, I2C_ID_FAR, I2C_ID_SPEED, I2C_ID_STOP, I2C_ID_FORWARD, I2C_ID_BACKWARD, I2C_ID_LEFT, I2C_ID_RIGHT, I2C_ID_HORN, I2C_ID_VOICE1, I2C_ID_VOICE2, I2C_ID_VOICE3, I2C_ID_VOL, I2C_ID_VOLUP, I2C_ID_VOLDOWN, I2C_ID_COLOR_R, I2C_ID_COLOR_G, I2C_ID_COLOR_B, I2C_ID_DIST, I2C_ID_MAX};

typedef struct I2C_PARAM {
    uint8_t id;
    uint8_t val;
};
typedef struct I2C_INFO{
    I2C_PARAM data[I2C_PARAM_SIZE];
    uint8_t size = 0;
};
typedef struct I2C_APP{
    I2C_PARAM data[I2C_ID_MAX];
    uint8_t size = 0;
};


class I2C_Extension : public TwoWire {
    public:
        // Initialization
        using TwoWire::TwoWire;
        void setup(uint8_t pinSDA, uint8_t pinSCL);

        // Communication
#ifdef I2C_MASTER
        bool sendAfterDataConversion(uint8_t address);
        bool sendData(uint8_t address, uint8_t* data, uint8_t length = 1);
        bool rcvData(uint8_t address);
#else
        bool sendData(void);
        bool sendData(uint8_t* data, uint8_t length = 1);
        bool rcvData(uint8_t length);
#endif

        // Data conversion
        //  - App to I2C
        bool setSendInfo( I2C_INFO info );
        bool setSendInfo( I2C_COM_ID id, uint8_t val );

        //  - Info to App
        void updateRcv(void);

        // for Application
        inline I2C_INFO getRcvInfo(void) { return m_rcvInfo; }   // from HTTP (to I2C)
        inline uint8_t getVal(I2C_COM_ID id) { return m_rcvApp.data[id].val; }
        inline uint8_t getSendInfoSize(void) { return m_sendInfo.size; }
        inline bool hasUpdated(I2C_COM_ID id) { return m_hasUpdated[id]; }
        inline bool hasNonZero(I2C_COM_ID id) { return m_hasNonZero[id]; }
        void printBytes(uint8_t* bytes, uint8_t length);
        void printSendInfo(void);
        void printRcvInfo(void);
        void printRcvApp(void);
        template<class T> void printVal(T info, const char* msg);

    private:
        // Initialization
        uint8_t m_pinSDA = 0;
        uint8_t m_pinSCL = 0;

        // Communication
        I2C_INFO m_sendInfo;    // to I2C
        I2C_INFO m_rcvInfo;     // from I2C

        // for Application
        I2C_APP  m_rcvApp;
        bool m_hasUpdated[I2C_ID_MAX] = {false};    // 1:If the value change, 0:If not (Initialize for each loop)
        bool m_hasNonZero[I2C_ID_MAX] = {false};    // 1:If the value is 1,   0:If not (Initialize for each loop)
};

#endif
