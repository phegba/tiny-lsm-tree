#include "../../include/skiplist/skiplist.h"
#include <cstdint>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace tiny_lsm 
{

// ************************ SkipListIterator ************************
BaseIterator &SkipListIterator::operator++()
 {
  // TODO: Lab1.2 任务：实现SkipListIterator的++操作符
   if(current)
   {
     current = current->forward_[0];
   }
  return *this;
}

bool SkipListIterator::operator==(const BaseIterator &other) const 
{
  // TODO: Lab1.2 任务：实现SkipListIterator的==操作符
  if (other.get_type() != IteratorType::SkipListIterator)
  {
    return false;
  }
  auto other2 = dynamic_cast<const SkipListIterator &>(other);
  return current == other2.current;
}

bool SkipListIterator::operator!=(const BaseIterator &other) const {
  // TODO: Lab1.2 任务：实现SkipListIterator的!=操作符
  if (other.get_type() != IteratorType::SkipListIterator)
  {
    return false;
  }
  auto other2 = dynamic_cast<const SkipListIterator &>(other);
  return current != other2.current;
}

SkipListIterator::value_type SkipListIterator::operator*() const 
{
  // TODO: Lab1.2 任务：实现SkipListIterator的*操作符
  if (!current)
    throw std::runtime_error("Dereferencing invalid iterator");
  return {current->key_, current->value_};
}

IteratorType SkipListIterator::get_type() const 
{
  // TODO: Lab1.2 任务：实现SkipListIterator的get_type
  // 主要是为了熟悉基类的定义和继承关系
  return IteratorType::SkipListIterator;
}

bool SkipListIterator::is_valid() const
 {
  return current && !current->key_.empty();
}
bool SkipListIterator::is_end() const { return current == nullptr; }
std::string SkipListIterator::get_key() const { return current->key_; }
std::string SkipListIterator::get_value() const { return current->value_; }
uint64_t SkipListIterator::get_tranc_id() const { return current->tranc_id_; }

// ************************ SkipList ************************
// 构造函数
SkipList::SkipList(int max_lvl) : max_level(max_lvl), current_level(1) {
  head = std::make_shared<SkipListNode>("", "", max_level, 0);
  dis_01 = std::uniform_int_distribution<>(0, 1);
  dis_level = std::uniform_int_distribution<>(0, (1 << max_lvl) - 1);
  gen = std::mt19937(std::random_device()());
}

int SkipList::random_level() {
   int level=1;//定义层数,最初始起码有一层
 while(dis_01(gen)==0 && level<max_level)  // 通过"抛硬币"的方式随机生成层数：每次有50%的概率增加一层,层数范围限制在[1, max_level]之间，避免浪费内存
 {
      level=level+1;//  dis_01(gen)有50%概率返回ture,也就是说level小于maxlevel时有50%概率是进入下一层，这样第二层就占总体50%，第三层25%,以此类推。
 }
  // TODO: Lab1.1 任务：插入时随机为这一次操作确定其最高连接的链表层数
  return level;

}

// 插入或更新键值对
void SkipList::put(const std::string &key, const std::string &value,
                   uint64_t tranc_id) {
  spdlog::trace("SkipList--put({}, {}, {})", key, value, tranc_id);
  // TODO: Lab1.1  任务：实现插入或更新键值对
     if(value.empty())//如果值为空，抛出异常
     {
     throw std::runtime_error("value is empty");
     }
  // ? Hint: 你需要保证不同`Level`的步长从底层到高层逐渐增加
  auto current=head;
 std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);
  for(int i=current_level-1;i>=0;i--)//从最高层开始查找,注意第current层下标为current_level-1
  {
    // 向下遍历
  {
    while(current->forward_[i] && current->forward_[i]->key_<key)//直到遇到空指针或者大于目标值的键值
    {
      current=current->forward_[i];//current指针指向同层下一个目标
    }
    update[i]=current;
  }
}
    //移动到最底层
  current=current->forward_[0];
    //判断key是否存在
  if(current && current->key_==key)
  {
    size_bytes+=value.size()-current->value_.size();//计算新value大小和旧value大小之差并且累加到总大小
     current->value_=value;
     return;
  }
  //value 不存在则需要创建
  int newlevel=random_level();
  //如果新层数比当前层数高，则需要更新指针
  if(newlevel>current_level)
  {
    for(int i=current_level;i<newlevel;i++)
    {
      update[i]=head;
    }
    current_level=newlevel;
  }
  //创建新节点
  auto new_node=std::make_shared<SkipListNode>(key,value,newlevel,tranc_id);
  //更新内存
  size_bytes+=new_node->key_.size()+new_node->value_.size()+ sizeof(uint64_t);
  //更新各层指针
  for(int i=0;i<newlevel;i++)
  {
    new_node->forward_[i]=update[i]->forward_[i];
    update[i]->forward_[i]=new_node;
  }
  //更新back_ward指针
  for(int i=0; i<newlevel; i++)  // 保持一致性
{
  new_node->set_backward(i, update[i]);
  if(new_node->forward_[i])
   {
     new_node->forward_[i]->set_backward(i, new_node);
  }
}
}

// 查找键值对
SkipListIterator SkipList::get(const std::string &key, uint64_t tranc_id) 
{
  // spdlog::trace("SkipList--get({}) called", key);
  // ? 你可以参照上面的注释完成日志输出以便于调试
  // ? 日志为输出到你执行二进制所在目录下的log文件夹

  // TODO: Lab1.1 任务：实现查找键值对,
    auto current = head;
     // 从最高层开始向下搜索
  for (int i = current_level - 1; i >= 0; i--)
   {
    // 在当前层向右移动，直到找到大于等于目标key的节点或到达末尾
    while (current->forward_[i] && current->forward_[i]->key_ < key)
     {
      current = current->forward_[i];
    }
  }
  // 移动到最底层的下一个节点
  current = current->forward_[0];
  // 检查是否找到了匹配的key
  if (current && current->key_ == key) 
  {
      return SkipListIterator(current);
  }
  // TODO: 并且你后续需要额外实现SkipListIterator中的TODO部分(Lab1.2)
  return SkipListIterator{};
}

// 删除键值对
// ! 这里的 remove 是跳表本身真实的 remove,  lsm 应该使用 put 空值表示删除,
// ! 这里只是为了实现完整的 SkipList 不会真正被上层调用
void SkipList::remove(const std::string &key)
 {
  // TODO: Lab1.1 任务：实现删除键值对
   std::vector<std::shared_ptr<SkipListNode>> update(max_level, nullptr);
   auto current=head;
    for(int i=current_level-1;i>=0;i--)//从最高层开始查找,注意第current层下标为current_level-1
  {
     while(current->forward_[i] && current->forward_[i]->key_<key)//直到遇到空指针或者等于目标值的键值
    {
      current=current->forward_[i];//current指针指向同层下一个目标
    }
    update[i] = current;
  }
   //移动到最底层
  current=current->forward_[0];
  //如果找到目标节点，执行删除操作
if(current && current->key_==key)
{
  for(int i=0;i<current_level;i++)
  {
    if(update[i]->forward_[i]!=current)
    {
      break;
    }
    // 更新前向指针
    update[i]->forward_[i]=current->forward_[i];
    // 更新后向指针
    if (current->forward_[i]) 
    {
       current->forward_[i]->backward_[i] = update[i];
    }
  }
  //更新内存
  size_bytes-=current->key_.size()+current->value_.size()+ sizeof(uint64_t);
  //如果删除的是最高层节点，需要维护跳表层级
  while(current_level>1 && head->forward_[current_level-1]==nullptr)
  {
    current_level--;
  }
}

}
// 刷盘时可以直接遍历最底层链表
std::vector<std::tuple<std::string, std::string, uint64_t>> SkipList::flush() {
  // std::shared_lock<std::shared_mutex> slock(rw_mutex);
  spdlog::debug("SkipList--flush(): Starting to flush skiplist data");

  std::vector<std::tuple<std::string, std::string, uint64_t>> data;
  auto node = head->forward_[0];
  while (node) {
    data.emplace_back(node->key_, node->value_, node->tranc_id_);
    node = node->forward_[0];
  }

  spdlog::debug("SkipList--flush(): Flushed {} entries", data.size());

  return data;
}

size_t SkipList::get_size() {
  // std::shared_lock<std::shared_mutex> slock(rw_mutex);
  return size_bytes;
}

// 清空跳表，释放内存
void SkipList::clear() {
  // std::unique_lock<std::shared_mutex> lock(rw_mutex);
  head = std::make_shared<SkipListNode>("", "", max_level, 0);
  size_bytes = 0;
}

SkipListIterator SkipList::begin() {
  // return SkipListIterator(head->forward[0], rw_mutex);
  return SkipListIterator(head->forward_[0]);
}

SkipListIterator SkipList::end() {
  return SkipListIterator(); // 使用空构造函数
}

// 找到前缀的起始位置
// 返回第一个前缀匹配或者大于前缀的迭代器
SkipListIterator SkipList::begin_preffix(const std::string &preffix) {
  // TODO: Lab1.3 任务：实现前缀查询的起始位置
  auto current=head;
      // 从最高层开始向下搜索
  for (int i = current_level - 1; i >= 0; i--)
   {
    // 在当前层向右移动，直到找到大于等于目标key的节点或到达末尾
    while (current->forward_[i] && current->forward_[i]->key_ < preffix)
     {
      current = current->forward_[i];
    }
  }
  // 移动到最底层的下一个节点
  current = current->forward_[0];
  // 检查前缀是否匹配
  if (current && current->key_ == preffix) 
  {
      spdlog::trace("SkipList--begin_preffix('{}'): first match at '{}'", preffix,
                  current->key_);
  }

    return SkipListIterator(current);
}

// 找到前缀的终结位置
SkipListIterator SkipList::end_preffix(const std::string &prefix) {
  // TODO: Lab1.3 任务：实现前缀查询的终结位置
    auto current=head;
      // 从最高层开始向下搜索
  for (int i = current_level - 1; i >= 0; i--)
   {
    // 在当前层向右移动，直到找到大于等于目标key的节点或到达末尾
    while (current->forward_[i] && current->forward_[i]->key_ < prefix)
     {
      current = current->forward_[i];
    }
  }
  // 移动到最底层的下一个节点
  current = current->forward_[0];
  // 2. 继续向后查找，直到找到第一个不匹配 prefix 的节点
    while ( current && current->key_.substr(0, prefix.size() )== prefix) 
    {
        current = current->forward_[0];
    }
// 此时 current 是第一个不匹配 prefix 的节点（即 end_prefix）
    if (current) 
    {
        spdlog::trace("SkipList--end_prefix('{}'): ends at '{}'", prefix, current->key_);
    } else {
        spdlog::trace("SkipList--end_prefix('{}'): ends at end()", prefix);
    }
  

    return SkipListIterator(current);
}

// ? 这里单调谓词的含义是, 整个数据库只会有一段连续区间满足此谓词
// ? 例如之前特化的前缀查询，以及后续可能的范围查询，都可以转化为谓词查询
// ? 返回第一个满足谓词的位置和最后一个满足谓词的迭代器
// ? 如果不存在, 范围nullptr
// ? 谓词作用于key, 且保证满足谓词的结果只在一段连续的区间内, 例如前缀匹配的谓词
// ? predicate返回值:
// ?   0: 满足谓词
// ?   >0: 不满足谓词, 需要向右移动
// ?   <0: 不满足谓词, 需要向左移动
// ! Skiplist 中的谓词查询不会进行事务id的判断, 需要上层自己进行判断

std::optional<std::pair<SkipListIterator, SkipListIterator>>
SkipList::iters_monotony_predicate(
    std::function<int(const std::string &)> predicate) {
    
  // 第一步：找到第一个满足谓词的节点
  auto current = head;
  
  // 使用跳表的层级结构来快速定位
  for (int level = current_level - 1; level >= 0; level--) {
    while (current->forward_[level]) {
      int result = predicate(current->forward_[level]->key_);
      
      if (result == 0) {
        // 找到满足谓词的节点，但可能不是第一个，继续检查左边是否有也满足的
        break;
      } else if (result < 0) {
        // 需要向左移动（当前节点太大），停止在这一层的搜索
        break;
      } else {
        // 需要向右移动（当前节点太小）
        current = current->forward_[level];
      }
    }
  }
  
  // 在最底层精确查找第一个满足谓词的节点
  while (current->forward_[0]) {
    int result = predicate(current->forward_[0]->key_);
    if (result == 0) {
      // 找到第一个满足谓词的节点
      break;
    } else if (result < 0) {
      // 需要向左移动，说明不存在满足谓词的节点
      return std::nullopt;
    } else {
      // 需要向右移动
      current = current->forward_[0];
    }
  }
  
  // 检查是否找到了满足谓词的节点
  if (!current->forward_[0]) {
    return std::nullopt;
  }
  
  // first_match 是第一个满足谓词的节点
  auto first_match = current->forward_[0];
  
  // 第二步：找到最后一个满足谓词的节点
  auto last_match = first_match;
  while (last_match->forward_[0]) {
    int result = predicate(last_match->forward_[0]->key_);
    if (result == 0) {
      // 继续向右寻找
      last_match = last_match->forward_[0];
    } else {
      // 不再满足谓词，last_match 就是最后一个满足谓词的节点
      break;
    }
  }
  
  // 返回 [begin_iter, end_iter) 迭代器对
  SkipListIterator begin_iter(first_match);
  SkipListIterator end_iter(last_match->forward_[0]); // 指向下一个节点（开区间）
  
  return std::make_pair(begin_iter, end_iter);
}

// ? 打印跳表, 你可以在出错时调用此函数进行调试
void SkipList::print_skiplist() 
{
  for (int level = 0; level < current_level; level++) {
    std::cout << "Level " << level << ": ";
    auto current = head->forward_[level];
    while (current) {
      std::cout << current->key_;
      current = current->forward_[level];
      if (current) {
        std::cout << " -> ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
} // namespace tiny_lsm