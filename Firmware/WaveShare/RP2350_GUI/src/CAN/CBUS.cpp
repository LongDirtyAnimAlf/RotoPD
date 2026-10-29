// CBUS library
#include "./CBUS.h"

CBUSbase::CBUSbase() {
}

inline byte CBUSbase::getCANID(unsigned long header) {
  return header & 0x7f;
}

bool CBUSbase::isExt(CANFrame *amsg) {
  return (amsg->ext);
}

//
/// is this a Remote frame ?
//

bool CBUSbase::isRTR(CANFrame *amsg) {

  return (amsg->rtr);
}

CBUScoe::CBUScoe(const byte num_items) {

  coe_buff = new circular_buffer2(num_items);
}

CBUScoe::~CBUScoe() {

  free(coe_buff);
}

void CBUScoe::put(const CANFrame *msg) {

  coe_buff->put(msg);
}

bool CBUScoe::available(void) {

  return coe_buff->available();
}

CANFrame CBUScoe::get(void) {
  CANFrame msg;
  memcpy(&msg, coe_buff->get(), sizeof(CANFrame));
  return msg;
}

///
/// a circular buffer class
///

/// constructor and destructor

circular_buffer2::circular_buffer2(byte num_items) {

  _head = 0;
  _tail = 0;
  _hwm = 0;
  _capacity = num_items;
  _size = 0;
  _puts = 0;
  _gets = 0;
  _overflows = 0;
  _full = false;
  _buffer = (buffer_entry2_t *)malloc(num_items * sizeof(buffer_entry2_t));
}

circular_buffer2::~circular_buffer2() {
  free(_buffer);
}

/// if buffer has one or more stored items

bool circular_buffer2::available(void) {

  return (_size > 0);
}

/// store an item to the buffer - overwrite oldest item if buffer is full
/// never called from an interrupt context so we don't need to worry about interrupts

void circular_buffer2::put(const CANFrame *item) {

  memcpy((CANFrame*)&_buffer[_head]._item, (const CANFrame *)item, sizeof(CANFrame));
  _buffer[_head]._item_insert_time = micros();

  // if the buffer is full, this put will overwrite the oldest item

  if (_full) {
    _tail = (_tail + 1) % _capacity;
    ++_overflows;
  }

  _head = (_head + 1) % _capacity;
  _full = _head == _tail;
  _size = size();
  _hwm = (_size > _hwm) ? _size : _hwm;
  ++_puts;

  return;
}

/// retrieve the next item from the buffer

CANFrame *circular_buffer2::get(void) {

  CANFrame *p = nullptr;

  // should always call ::available first to avoid returning null pointer

  if (_size > 0) {
    p = &_buffer[_tail]._item;
    _full = false;
    _tail = (_tail + 1) % _capacity;
    _size = size();
    ++_gets;
  }

  return p;
}

/// get the insert time of the current buffer tail item
/// must be called before the item is removed by ::get

unsigned long circular_buffer2::insert_time(void) {

  return (_buffer[_tail]._item_insert_time);
}

/// peek at the next item in the buffer without removing it

CANFrame *circular_buffer2::peek(void) {

  // should always call ::available first to avoid this

  if (_size == 0) {
    return nullptr;
  }

  return (&_buffer[_tail]._item);
}

/// clear all items

void circular_buffer2::clear(void) {

  _head = 0;
  _tail = 0;
  _full = false;
  _size = 0;

  return;
}

/// return high water mark

byte circular_buffer2::hwm(void) {

  return _hwm;
}

/// return full indicator

bool circular_buffer2::full(void) {

  return _full;
}

/// recalculate number of items in the buffer

byte circular_buffer2::size(void) {

  byte size = _capacity;

  if (!_full) {
    if (_head >= _tail) {
      size = _head - _tail;
    } else {
      size = _capacity + _head - _tail;
    }
  }

  _size = size;
  return _size;
}

/// return empty indicator

bool circular_buffer2::empty(void) {

  return (!_full && (_head == _tail));
}

/// return number of free slots

byte circular_buffer2::free_slots(void) {

  return (_capacity - _size);
}

/// number of puts

unsigned int circular_buffer2::puts(void) {

  return _puts;
}

/// number of gets

unsigned int circular_buffer2::gets(void) {

  return _gets;
}

/// number of overflows

unsigned int circular_buffer2::overflows(void) {

  return _overflows;
}

