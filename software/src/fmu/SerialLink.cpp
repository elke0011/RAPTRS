/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor
*/

#include "SerialLink.h"

/* Assigning a hardware serial bus */
SerialLink::SerialLink(HardwareSerial& bus)
{
  _bus = &bus;
}
/*
* Starting the bus at the specified baud rate.
*/
void SerialLink::begin(unsigned int baud)
{
  _bus->begin(baud);
}
/*
* Starting to build a new packet to send.
*/
void SerialLink::beginTransmission(MsgType type)
{
  /* resetting payload length */
  _payload_len = 0;
  /* resetting the frame position */
  _send_fpos = 0;
  /* framing byte */
  _send_buf[_send_fpos++] = _frame_byte;
  /* control byte */
  _send_buf[_send_fpos++] = type;
  _send_crc = _send_crc_16.xmodem(&_send_buf[1],_send_fpos - 1);
}
/*
* Add a byte to the send buffer.
*/
unsigned int SerialLink::write(unsigned char data)
{
  if (_payload_len < _max_payload_len) {
    _send_crc = _send_crc_16.xmodem_upd(&data,1);
    if ((data == _frame_byte) || (data == _esc_byte)) {
      _send_buf[_send_fpos++] = _esc_byte;
      _send_buf[_send_fpos++] = data ^ _invert_byte;
    } else {
      _send_buf[_send_fpos++] = data;
    }
    _payload_len++;
    return 1;
  } else {
    return 0;
  }
}
/*
* Add data bytes to the send buffer.
*/
unsigned int SerialLink::write(unsigned char *data, unsigned int len)
{
  unsigned int avail = _max_payload_len - _payload_len;
  if (len > avail) {
    len = avail;
  }
  for (unsigned int i = 0; i < len; i++) {
    _send_crc = _send_crc_16.xmodem_upd(&data[i],1);
    if ((data[i] == _frame_byte) || (data[i] == _esc_byte)) {
      _send_buf[_send_fpos++] = _esc_byte;
      _send_buf[_send_fpos++] = data[i] ^ _invert_byte;
    } else {
      _send_buf[_send_fpos++] = data[i];
    }
  }
  return len;
}
/*
* Send packet without waiting for ack.
*/
void SerialLink::sendTransmission()
{
  unsigned char crc_bytes[2];
  /* crc */
  crc_bytes[0] = _send_crc & 0xFF;
  crc_bytes[1] = (_send_crc >> 8) & 0xFF;
  for (unsigned int i = 0; i < ARRAY_SIZE(crc_bytes); ++i) {
    if ((crc_bytes[i] == _frame_byte) || (crc_bytes[i] == _esc_byte)) {
      _send_buf[_send_fpos++] = _esc_byte;
      _send_buf[_send_fpos++] = crc_bytes[i] ^ _invert_byte;
    } else {
      _send_buf[_send_fpos++] = crc_bytes[i];
    }
  }
  /* framing byte */
  _send_buf[_send_fpos++] = _frame_byte;
  /* update send status */
  _status = NACK;
  /* write frame */
  _bus->write(_send_buf,_send_fpos);
}
/*
* Send packet and wait indefinitely for ack.
*/
void SerialLink::endTransmission(bool ackReq)
{
  /* Send packet */
  sendTransmission();

  if (ackReq) { /* wait for ACK */
    while (_status != ACK) {
      checkReceived();
    }
  } else { /* maintain the status flag */
    _status = ACK;
  }
}
/*
* Send packet and wait for ack or timeout.
*/
void SerialLink::endTransmission(unsigned int timeout)
{
  /* Send packet */
  sendTransmission();
  /* wait for ACK */
  elapsedMicros t = 0;
  while ((_status != ACK) && (t < timeout)) {
    checkReceived();
  }
}
/*
* Check to see if we've received any new messages.
*/
bool SerialLink::checkReceived()
{
  int c;
  unsigned short crc;
  while (_bus->available()) {
    c = _bus->read();
    /* frame start */
    if (_recv_fpos == 0) {
      if (c == _frame_byte) {
        _recv_buf[_recv_fpos++] = c;
      }
    } else {
      if (c == _frame_byte) {
        /* frame end */
        if (_recv_fpos == 1) {
          // Do nothing
        } else if (_recv_fpos >= _header_len + _footer_len - 1) {
          /* passed crc check, good packet */
          crc = _recv_crc_16.xmodem(&_recv_buf[1],_recv_fpos - 3);
          if (crc == (((unsigned short)_recv_buf[_recv_fpos-1] << 8) | _recv_buf[_recv_fpos - 2])) {
            _msg_len = _recv_fpos - _header_len - _footer_len + 1; // +1 because we didn't step fpos
            _read_pos = _header_len;
            _recv_fpos = 0;
            _escape = false;
            _recv_type = (MsgType)_recv_buf[1];
            if ((_recv_type == ACK) || (_recv_type == NACK)) {
              _status = _recv_type;
              return false;
            }
            return true;
          /* did not pass crc, bad packet */
          } else {
            _recv_fpos = 0;
            _escape = false;
            return false;
          }
        /* bad frame */
        } else {
          _recv_fpos = 0;
          _escape = false;
          return false;
        }
      } else if (c == _esc_byte) {
        _escape = true;
      } else if (_escape) {
        c = c ^ _invert_byte;
        _escape = false;
        /* read into buffer */
        _recv_buf[_recv_fpos++] = c;
      /* prevent buffer overflow */
      } else if (_recv_fpos >= BUFFER_SIZE) {
        _recv_fpos = 0;
        _escape = false;
      } else {
        /* read into buffer */
        _recv_buf[_recv_fpos++] = c;
      }
    }
  }
  return false;
}
/*
* How many bytes available in our RX buffer
*/
unsigned int SerialLink::available()
{
  return _msg_len;
}
/*
* Read a byte
*/
unsigned char SerialLink::read()
{
  if (_msg_len > 0) {
    _msg_len--;
    return _recv_buf[_read_pos++];
  } else {
    return 0;
  }
}
/*
* Read several bytes
*/
unsigned int SerialLink::read(unsigned char *data, unsigned int len)
{
  if (len > _msg_len) {
    len = _msg_len;
  }
  memcpy(data,&_recv_buf[_read_pos],len);
  _read_pos += len;
  _msg_len -=len;
  return len;
}
/*
* Send ack or nack response
*/
void SerialLink::sendStatus(bool ack)
{
  /* control byte */
  MsgType type;
  type = ack ? ACK : NACK;

  beginTransmission(type);
  sendTransmission();
}
/*
* Get ack or nack status
*/
bool SerialLink::getTransmissionStatus()
{
  return (_status == ACK) ? true : false;
}
