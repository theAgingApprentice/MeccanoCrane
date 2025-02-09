#include "MqttLogger.h" // For logging to serial and/or MQTT.
#include "Arduino.h" // Arduino Core for ESP32. Comes with PlatformIO.

/**
 * @brief Construct a new Mqtt Logger:: Mqtt Logger object
 * 
 * @param mode The mode of the logger.
 * 
 * @return NA No return value.
 */
MqttLogger::MqttLogger(MqttLoggerMode mode)
{
    this->setMode(mode);
    this->setBufferSize(MQTT_MAX_PACKET_SIZE);
} // MqttLogger::MqttLogger()

/**
 * @brief Construct a new Mqtt Logger:: Mqtt Logger object
 * 
 * @param client The PubSubClient object.
 * @param topic The topic to publish logs to.
 * @param mode The mode of the logger.
 * @param retained Whether the message should be retained.
 * 
 * @return NA No return value.
 */
MqttLogger::MqttLogger(PubSubClient& client, const char* topic, MqttLoggerMode mode, const boolean& retained)
{
    this->setClient(client);
    this->setTopic(topic);
    this->setMode(mode);
    this->setBufferSize(MQTT_MAX_PACKET_SIZE);
    this->setRetained(retained);
} // MqttLogger::MqttLogger()

/**
 * @brief Destroy the Mqtt Logger:: Mqtt Logger object
 * 
 * @param NA No parameters.
 * 
 * @return NA No return value.
 */
MqttLogger::~MqttLogger()
{
} // MqttLogger::~MqttLogger()

/**
 * @brief Set the PubSubClient object
 * 
 * @param client The PubSubClient object.
 * 
 * @return NA No return value.
 */
void MqttLogger::setClient(PubSubClient& client)
{
    this->client = &client;
} // MqttLogger::setClient()

/**
 * @brief Set the topic to publish logs to.
 * 
 * @param topic The topic to publish logs to.
 * 
 * @return NA No return value.
 */
void MqttLogger::setTopic(const char* topic)
{
    this->topic = topic;
} // MqttLogger::setTopic()

/**
 * @brief Set the mode of the WiFi client.
 * 
 * @details Valid modes for a WiFi client are:
 * 1. WIFI_MODE_NULL : The null mode or the WIFI_MODE_OFF which is the OFF 
 *    mode.
 * 2. WIFI_MODE_STA : The Station mode, which is the standard client mode.
 * 3. WIFI_MODE_AP : The Access Point mode where clients can connect to the 
 *    ESP32.
 * 4. WIFI_MODE_APSTA : The hybrid mode where the ESP connect to an AP as a 
 *    client and recieve connections from client as an AP.
 * 5. WIFI_MODE_MAX : The MAX mode has no explaination on what it is supposed 
 *    to do. But it dose not seems to be implemented anyway.
 * 
 * @param mode The mode to put the WiFi client into.
 * 
 * @return NA No return value.
 */
void MqttLogger::setMode(MqttLoggerMode mode)
{
    this->mode = mode;
} // MqttLogger::setMode()

/**
 * @brief Set message retention rule for broker.
 * 
 * @param retained True to retain messages and False to not.
 * 
 * @return NA No return value.
 */
void MqttLogger::setRetained(const boolean& retained)
{
    this->retained = retained;
} // MqttLogger::setRetained()

/**
 * @brief Set message buffer size.
 * 
 * @param NA No parameters.
 * 
 * @return NA No return value.
 */
uint16_t MqttLogger::getBufferSize()
{
    return this->bufferSize;
} // MqttLogger::getBufferSize())

/**
 * @brief (Re)allocate local buffer and reset end-to-start of buffer.
 * 
 * @param NA No parameters.
 * 
 * @return NA No return value.
 */
boolean MqttLogger::setBufferSize(uint16_t size)
{
    if (size == 0)
    {
        return false;
    } // if
    if (this->bufferSize == 0)
    {
        this->buffer = (uint8_t *)malloc(size);
        this->bufferEnd = this->buffer;
    } // if
    else
    {
        uint8_t *newBuffer = (uint8_t *)realloc(this->buffer, size);
        if (newBuffer != NULL)
        {
            this->buffer = newBuffer;
            this->bufferEnd = this->buffer;
        } // if
        else
        {
            return false;
        } // else
    } // else
    this->bufferSize = size;
    return (this->buffer != NULL);
} // MqttLogger::setBufferSize()

/**
 * @brief Send & reset current buffer.
 * 
 * @param NA No parameters.
 * 
 * @return NA No return value.
 */
void MqttLogger::sendBuffer() 
{
    if (this->bufferCnt > 0)
    {
        bool doSerial = this->mode==MqttLoggerMode::SerialOnly || this->mode==MqttLoggerMode::MqttAndSerial;
        if (this->mode!=MqttLoggerMode::SerialOnly && this->client != NULL && this->client->connected()) 
        {
            this->client->publish(this->topic, (byte *)this->buffer, this->bufferCnt, retained);
        } // if 
        else if (this->mode == MqttLoggerMode::MqttAndSerialFallback)
        {
            doSerial = true;
        } //  else if
        if (doSerial) 
        {
            Serial.write(this->buffer, this->bufferCnt);
            Serial.println();
        } // if
        this->bufferCnt=0;
    } // if
    this->bufferEnd=this->buffer;
} // MqttLogger::sendBuffer()

/**
 * @brief implement Print::write(uint8_t c): store into a buffer until \n or 
 *        buffer full.
 * 
 * @param character current character of  buffer to transmit.
 * 
 * @return 1 Always return the value 1.
 */
size_t MqttLogger::write(uint8_t character)
{
    if (character == '\n') // when newline is printed we send the buffer
    {
        this->sendBuffer();
    } // if
    else
    {
        if (this->bufferCnt < this->bufferSize) // add char to end of buffer
        {
            *(this->bufferEnd++) = character;
            this->bufferCnt++;
        } // if
        else // buffer is full, first send&reset buffer and then add char to buffer
        {
            this->sendBuffer();
            *(this->bufferEnd++) = character;
            this->bufferCnt++;
        } //  else
    } // else
    return 1;
} // MqttLogger::write()