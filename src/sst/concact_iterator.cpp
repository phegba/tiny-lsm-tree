#include "../../include/sst/concact_iterator.h"

namespace tiny_lsm {

ConcactIterator::ConcactIterator(std::vector<std::shared_ptr<SST>> ssts,
                                 uint64_t tranc_id)
    : ssts(ssts), cur_iter(nullptr, tranc_id), cur_idx(0),
      max_tranc_id_(tranc_id) {
  if (!this->ssts.empty()) {
    cur_iter = ssts[0]->begin(max_tranc_id_);
  }
}

BaseIterator &ConcactIterator::operator++() {
  // TODO: Lab 4.3 自增运算符重载
  return *this;
}

bool ConcactIterator::operator==(const BaseIterator &other) const {
  // TODO: Lab 4.3 比较运算符重载
  return false;
}

bool ConcactIterator::operator!=(const BaseIterator &other) const {
  // TODO: Lab 4.3 比较运算符重载
  return false;
}

ConcactIterator::value_type ConcactIterator::operator*() const {
  // TODO: Lab 4.3 解引用运算符重载
  return value_type();
}

IteratorType ConcactIterator::get_type() const {
  return IteratorType::ConcactIterator;
}

uint64_t ConcactIterator::get_tranc_id() const { return max_tranc_id_; }

bool ConcactIterator::is_end() const {
  return cur_iter.is_end() || !cur_iter.is_valid();
}

bool ConcactIterator::is_valid() const {
  return !cur_iter.is_end() && cur_iter.is_valid();
}

ConcactIterator::pointer ConcactIterator::operator->() const {
  // TODO: Lab 4.3 ->运算符重载
  return nullptr;
}

std::string ConcactIterator::key() { return cur_iter.key(); }

std::string ConcactIterator::value() { return cur_iter.value(); }
} // namespace tiny_lsm