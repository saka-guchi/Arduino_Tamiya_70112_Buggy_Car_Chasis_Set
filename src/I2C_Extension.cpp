#include "Arduino.h"
#include "I2C_Extension.h"

void I2C_Extension::setup(uint8_t pinSDA, uint8_t pinSCL) {
    m_pinSDA = pinSDA;
    m_pinSCL = pinSCL;

    Wire.begin();

    delay(500);
    Serial.println(F("I2C:\t\tSuccessfully initialized."));
}

bool I2C_Extension::setSendInfo( I2C_INFO info ) {
    if( m_sendInfo.size >= I2C_PARAM_SIZE ) {
        return false;
    }
//  debugPrint("Send to I2C (info.size = %d)\n", info.size);
    for(int i = 0; i < info.size; i++ ) {
        setSendInfo( (I2C_COM_ID)info.data[i].id, info.data[i].val );
    }
    return true;
}

bool I2C_Extension::setSendInfo( I2C_COM_ID id, uint8_t val ) {
    if( m_sendInfo.size >= I2C_PARAM_SIZE ) {
        return false;
    }
//  debugPrint("Send to I2C (id = %d, val = %d)\n", (uint8_t)id, val);
    m_sendInfo.data[m_sendInfo.size].id  = id;
    m_sendInfo.data[m_sendInfo.size].val = val;
    m_sendInfo.size++;
    return true;
}

void I2C_Extension::updateRcv(void) {
    memset( m_hasUpdated, 0, sizeof(m_hasUpdated) );
    memset( m_hasNonZero, 0, sizeof(m_hasNonZero) );
    if( m_rcvInfo.size == 0 ) {
        return;
    }

    for(int i = 0; i < m_rcvInfo.size; i++ ) {
        m_rcvApp.data[m_rcvInfo.data[i].id].val = m_rcvInfo.data[i].val;
        m_hasUpdated[m_rcvInfo.data[i].id] = true;
        if(m_rcvApp.data[m_rcvInfo.data[i].id].val != 0) {
            m_hasNonZero[m_rcvInfo.data[i].id] = true;
        }
        Serial.print(F("updateRcv: (id = "));
        Serial.print(m_rcvInfo.data[i].id);
        Serial.print(F(", val = "));
        Serial.print(m_rcvApp.data[m_rcvInfo.data[i].id].val);
        Serial.println(F(") {0:DOOR, 1-3:RANGE, 4:SPEED, 5-9:MOVE, 10:HORN, 11-13:VOICE, 14-16:VOL, 17-19:COLOR, 20:DIST}"));
    }
    m_rcvInfo.size = 0;

    Serial.print("updateRcv: (Updated[id] = {");
    for(int i = 0; i < I2C_ID_MAX; i++ ) {
        Serial.print(" ");
        Serial.print( (m_hasUpdated[i])? 1:0 );
    }
    Serial.println("})");
    Serial.print("updateRcv: (NonZero[id] = {");
    for(int i = 0; i < I2C_ID_MAX; i++ ) {
        Serial.print(" ");
        Serial.print( (m_hasNonZero[i])? 1:0 );
    }
    Serial.println("})");
}

#ifdef I2C_MASTER
bool I2C_Extension::sendAfterDataConversion(uint8_t address) {
    if( m_sendInfo.size == 0 ) {
        return true;
    }
//  debugPrint("Converts data from Info to 1byte array (m_sendInfo.size = %d)\n", m_sendInfo.size );
    uint8_t data[m_sendInfo.size << 1] = {0};   // 2byte -> 1byte
    int count = 0;
    for(int i = 0; i < m_sendInfo.size; i++) {
        data[count] = m_sendInfo.data[i].id;
        count++;
        data[count] = m_sendInfo.data[i].val;
        count++;
    }
    m_sendInfo.size = 0;
    return sendData( address, data, count);
}

bool I2C_Extension::sendData(uint8_t address, uint8_t* data, uint8_t length) {
    if( length == 0 ) {
        return true;
    }
    debugPrint("Data transmission preparation (address = 0x%x, length = %d, data = {", address, length );
    Serial.print(data[0]);
    for( int i = 1; i < length; i++) {
        Serial.print(",");
        Serial.print(data[i]);
    }
    Serial.println("})");
    
    // Send: Master -> Slave
    Wire.beginTransmission(address);            // Address: 7bit(0-127)
    int sendByte = Wire.write(data, length);    // Data: 8bit array(0-255), Length: 8bit(0-255)
    int result = Wire.endTransmission(true);    // true: send stop (default), false: keeping the connection (when multiple masters)

    // Determine success
    bool ret = false;
    if( result == 0 ) {
        ret = true;
    } else {
        ret = false;
    }

    // Debug
    switch(result) {
        case 0:  debugPrint("<SUCCESS> Transmission to I2C slave is complete (bytes = %d)\n", sendByte); break;
        case 1:  debugPrint("<ERROR> Data too long to fit in transmit buffer.\n"); break;
        case 2:  debugPrint("<ERROR> Rcvd NACK on transmit of address.\n"); break;
        case 3:  debugPrint("<ERROR> Rcvd NACK on transmit of data.\n"); break;
        case 4:  debugPrint("<ERROR> Other error.\n"); break;
        default: debugPrint("<ERROR> Unknown error.\n"); break;
    }
    return ret;
}

bool I2C_Extension::rcvData(uint8_t address) {
    uint8_t size = ((uint8_t)I2C_REQUEST_SIZE << 1);    // 2byte unit
    uint8_t length = Wire.requestFrom(address, size);

//  debugPrint("requestFrom: (slave address = 0x%x, req size = %d -> return size = %d)\n", address, size, length);
    if( length != size ) {
        return false;
    }

    m_rcvInfo.size = I2C_REQUEST_SIZE;
    if( Wire.available() >= length ) {
        for( int i = 0; i < (length >> 1); i++ ) {
            m_rcvInfo.data[i].id  = Wire.read();
            m_rcvInfo.data[i].val = Wire.read();
        }
    }

    return true;
}

#else
bool I2C_Extension::sendData(void) {
    if( m_sendInfo.size == 0 ) {
        return true;
    }
//  Serial.print("sendData: (m_sendInfo.size = ");
//  Serial.print( m_sendInfo.size );
//  Serial.println(")");
    uint8_t data[m_sendInfo.size << 1] = {0};   // 2byte -> 1byte
    int count = 0;
    for(int i = 0; i < m_sendInfo.size; i++) {
        data[count] = m_sendInfo.data[i].id;
        count++;
        data[count] = m_sendInfo.data[i].val;
        count++;
    }
    m_sendInfo.size = 0;
    return sendData( data, count);
}

bool I2C_Extension::sendData(uint8_t* data, uint8_t length) {
    if( length == 0 ) {
        return false;
    }
    int sendByte = Wire.write(data, length);    // Data: 8bit array(0-255), Length: 8bit(0-255)
    Serial.print("sendData: <SUCCESS> Send to I2C master (bytes = ");
    Serial.print( sendByte );
    Serial.print(")\n");
    return true;
}

bool I2C_Extension::rcvData(uint8_t length) {
    for( int i = 0; i < length; i++ ) {
        uint8_t id  = Wire.read();
        uint8_t val = Wire.read();
        if( id == 255 ) { // 不定値255を利用してスキップ TODO:1byte目にサイズを貰う仕様に変更.
            continue;
        }
        if( m_rcvInfo.size < I2C_PARAM_SIZE) {
            m_rcvInfo.data[m_rcvInfo.size].id  = id;
            m_rcvInfo.data[m_rcvInfo.size].val = val;
            m_rcvInfo.size++;
        } else {
            // discard read data
            Serial.print("rcvData: <WARNING> Received data has been discarded (length = ");
            Serial.print(length);
            Serial.print(", I2C_PARAM_SIZE = ");
            Serial.print(I2C_PARAM_SIZE);
            Serial.print(")\n");
        }
    }
    return true;
}
#endif

void I2C_Extension::printRcvApp(void) {
    printVal(m_rcvApp, "printRcvApp: m_rcvApp[");
}

void I2C_Extension::printSendInfo(void) {
    printVal(m_sendInfo, "printSendInfo: Responsed data to I2C master (m_sendInfo[");
}

void I2C_Extension::printRcvInfo(void) {
    printVal(m_rcvInfo, "printRcvInfo: Received data from I2C master (m_rcvInfo[");
}

template          void I2C_Extension::printVal(I2C_INFO info, const char* msg);
template          void I2C_Extension::printVal(I2C_APP app, const char* msg);
template<class T> void I2C_Extension::printVal(T info, const char* msg) {
    Serial.print(msg);
    Serial.print(info.size);
    Serial.print("]{id:val} = [");
    for( int i = 0; i < info.size; i++ ) {
        Serial.print("{");
        Serial.print(info.data[i].id);
        Serial.print(",");
        Serial.print(info.data[i].val);
        Serial.print("}");
    }
    Serial.println("])");
}

void I2C_Extension::printBytes(uint8_t* bytes, uint8_t length) {
    Serial.print("printBytes: (Length = ");
    Serial.print(length);
    Serial.print(")\n");
    Serial.print("[uint] 0x");
    Serial.print( bytes[0], HEX );
    String str = "";
    str += (char)bytes[0];
    for( int i = 1; i < length; i++ ) {
        Serial.print(",0x");
        Serial.print( bytes[i], HEX );
        if( ( bytes[i] >= 0x20 ) && ( bytes[i] <= 0x7E ) ) {
            str += (char)bytes[i];  // ascii code only
        }
    }
    Serial.print("\n[char] \"");
    Serial.print(str);
    Serial.print("\"\n");
}