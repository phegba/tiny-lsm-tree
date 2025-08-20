#include "../../include/iterator/iterator.h"
#include <tuple>
#include <vector>

namespace tiny_lsm {

// *************************** SearchItem ***************************
bool operator<(const SearchItem &a, const SearchItem &b) {
  // TODO: Lab2.2 实现比较规则
   if (a.key_ != b.key_) 
   {
    return a.key_ < b.key_;
  }
  if (a.tranc_id_ > b.tranc_id_) 
  {
    return true;
  }
  if (a.level_ < b.level_) {
    return true;
  }
  return a.idx_ < b.idx_;
}

bool operator>(const SearchItem &a, const SearchItem &b) {
  // TODO: Lab2.2 实现比较规则
  if (a.key_ != b.key_) 
   {
    return a.key_ > b.key_;
  }
  if (a.tranc_id_ < b.tranc_id_) 
  {
    return true;
  }
  if (a.level_ > b.level_) {
    return true;
  }
  return a.idx_ > b.idx_;
}

bool operator==(const SearchItem &a, const SearchItem &b) {
  // TODO: Lab2.2 实现比较规则
  return a.idx_ == b.idx_ && a.key_ == b.key_;
}

// *************************** HeapIterator ***************************
HeapIterator::HeapIterator(std::vector<SearchItem> item_vec,
                           uint64_t max_tranc_id)
    : max_tranc_id_(max_tranc_id) {
  // TODO: Lab2.2 实现 HeapIterator 构造函数
   for(auto &item: item_vec)
   {
      items.push(item);
   }
   while (!items.empty() && items.top().value_.empty()) 
   {
      // 如果当前元素的value为空，则说明该元素已经被删除，需要从优先队列中删除
      auto del_key = items.top().key_;
      while (!items.empty() && items.top().key_ == del_key) {
        items.pop();
      }
    }
   
}

HeapIterator::pointer HeapIterator::operator->() const {
  // TODO: Lab2.2 实现 -> 重载
   update_current();
   return current.get();
}

HeapIterator::value_type HeapIterator::operator*() const {
  // TODO: Lab2.2 实现 * 重载
  return std::make_pair(items.top().key_, items.top().value_);
}

BaseIterator &HeapIterator::operator++() {
  // TODO: Lab2.2 实现 ++ 重载
  if (items.empty()) 
  {
    return *this; // 处理空队列情况
  }
   auto old_item = items.top();
   items.pop();
    while (!items.empty() && items.top().key_ == old_item.key_) 
  {
    items.pop();
  }
   while (!top_value_legal()) {
    // 1. 先跳过事务 id 不可见的部分
    skip_by_tranc_id();

    // 2. 跳过标记为删除的元素
    while (!items.empty() && items.top().value_.empty())
     {
      // 如果当前元素的value为空，则说明该元素已经被删除，需要从优先队列中删除
      auto del_key = items.top().key_;
      while (!items.empty() && items.top().key_ == del_key) 
      {
        items.pop();
      }
    }
  }
  return *this; 
}

bool HeapIterator::operator==(const BaseIterator &other) const {
  // TODO: Lab2.2 实现 == 重载
  if (other.get_type() != IteratorType::HeapIterator) 
  {
    return false;
  }
  auto other2=dynamic_cast<const HeapIterator&>(other);
  if (items.empty() || other2.items.empty()) 
{
    return items.empty() && other2.items.empty();
}
return items.top().key_ == other2.items.top().key_ &&  items.top().value_ == other2.items.top().value_;
}

bool HeapIterator::operator!=(const BaseIterator &other) const {
  // TODO: Lab2.2 实现 != 重载
  return !(*this == other);

}

bool HeapIterator::top_value_legal() const {
  // TODO: Lab2.2 判断顶部元素是否合法
  // ? 被删除的值是不合法
  // ? 不允许访问的事务创建或更改的键值对不合法(暂时忽略)
  if (items.empty()) 
  {
    return true;
  }

  if (max_tranc_id_ == 0) 
  {
    // 没有开启事务
    // 不为空的 value 才合法
    return items.top().value_.size() > 0;
  }

  return true;
}

void HeapIterator::skip_by_tranc_id() {
  // TODO: Lab2.2 后续的Lab实现, 只是作为标记提醒
}

bool HeapIterator::is_end() const { return items.empty(); }
bool HeapIterator::is_valid() const { return !items.empty(); }

void HeapIterator::update_current() const {
  // current 缓存了当前键值对的值, 你实现 -> 重载时可能需要
  // TODO: Lab2.2 更新当前缓存值
   if (!items.empty()) {
    current =
        std::make_shared<value_type>(items.top().key_, items.top().value_);
  } 
  else 
  {
    current.reset();
  }
}

IteratorType HeapIterator::get_type() const {
  return IteratorType::HeapIterator;
}

uint64_t HeapIterator::get_tranc_id() const { return max_tranc_id_; }
} // namespace tiny_lsm