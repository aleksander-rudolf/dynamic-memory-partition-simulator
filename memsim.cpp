#include "memsim.h"
#include <cassert>
#include <iostream>
#include <math.h>
#include <unordered_map>
#include <iterator>
#include <list>
#include <set>

struct Partition {
  int tag;
  int64_t size, addr;
};

typedef std::list<Partition>::iterator PartitionRef;
typedef std::set<PartitionRef>::iterator TreePartitionRef;

struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const {
    if(c1->size == c2->size)
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};

struct Simulator {
  int64_t page_size;
  int64_t n_pages_requested;
  //All partitions, in a linked list
  std::list<Partition> all_blocks;
  //Sorted partitions by size/address
  std::set<PartitionRef,scmp> free_blocks;
  //Quick access to all tagged partitions
  std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;

  Simulator(int64_t page_size) {
    //Constructor
    this->page_size = page_size;
    this->n_pages_requested = 0;
  }

  void allocate(int tag, int size) {

    //When all memory is free, allocate correct amount for the first partition.
    if(all_blocks.empty()){
      n_pages_requested += ceil((double)size/(double)this->page_size);
      all_blocks.push_back(Partition{-1, (int)ceil((double)size/(double)this->page_size)*this->page_size, 0});
      PartitionRef ref = all_blocks.begin();
      free_blocks.insert(ref);
    }

    //Find largest free parition
    PartitionRef ref;
    bool available_space = false;
    if(!free_blocks.empty()){
      TreePartitionRef treeRef = free_blocks.begin();
      if((*treeRef)->size >= size && (*treeRef)->tag == -1){
        ref = *treeRef;
        available_space = true;
      }
    }

    //When there is no free memory, get minimum number of pages from OS, but consider the
    //case when last partition is free. Add the new memory at the end of partition list.
    //The last partition will be the best partition.
    if(!available_space){ 
      
      if(std::prev(all_blocks.end())->tag == -1){
        free_blocks.erase(std::prev(all_blocks.end()));
        int64_t requested_memory = ceil(((double)size-(double)std::prev(all_blocks.end())->size)/(double)this->page_size)*this->page_size;
        n_pages_requested += ceil(((double)size-(double)std::prev(all_blocks.end())->size)/(double)this->page_size);
        std::prev(all_blocks.end())->size += requested_memory;
      }
      else{
        int64_t requested_memory = ceil((double)size/(double)this->page_size)*this->page_size;
        n_pages_requested += ceil((double)size/(double)this->page_size);
        all_blocks.push_back(Partition{-1, requested_memory, std::prev(all_blocks.end())->addr + std::prev(all_blocks.end())->size});
      }
      ref = std::prev(all_blocks.end());
      free_blocks.insert(std::prev(all_blocks.end()));
    }

    //Split the available partition in two if necessary. Mark the first partition occupied, 
    //and store the tag in it. Mark the second partition free.
    if(ref->tag == -1 && ref->size > size){
      free_blocks.erase(ref);
      int64_t free_space = ref->size-size;
      ref->tag = tag;
      tagged_blocks[tag].push_back(ref);
      ref->size = size;
      if(ref == std::prev(all_blocks.end())){
        all_blocks.push_back(Partition{-1, free_space, ref->addr+ref->size});
        free_blocks.insert(std::prev(all_blocks.end()));
      }
      else{
        all_blocks.insert(std::next(ref), Partition{-1, free_space, ref->addr+ref->size});
        free_blocks.insert(std::next(ref));
      }
    }
    else{
      free_blocks.erase(ref);
      ref->tag = tag;
      tagged_blocks[tag].push_back(ref);
      ref->size = size;
    } 
  }

  void deallocate(int tag) {

    std::vector<PartitionRef> iter = tagged_blocks[tag];

    //Iterate through every partition in tagged_blocks with a matching tag.
    for(int i = 0; i < (int)iter.size(); i++){
      PartitionRef curr_node = iter.at(i);
      //Mark the partition as free, and merge with any adjacent free partitions.
      curr_node->tag = -1;
      if(curr_node != all_blocks.begin() && std::prev(curr_node)->tag == -1){
        curr_node->size += std::prev(curr_node)->size;
        curr_node->addr = std::prev(curr_node)->addr;
        free_blocks.erase(std::prev(curr_node));
        all_blocks.erase(std::prev(curr_node));
        
      }
      if(curr_node != std::prev(all_blocks.end()) && std::next(curr_node)->tag == -1){
        curr_node->size += std::next(curr_node)->size;
        free_blocks.erase(std::next(curr_node));
        all_blocks.erase(std::next(curr_node));
      }
      //Update free_blocks
      free_blocks.insert(curr_node);
    }
    //Update tagged_blocks
    tagged_blocks.erase(tag);
  }

  MemSimResult getStats() {

    MemSimResult result;
    result.max_free_partition_size = 0;
    result.max_free_partition_address = 0;
    result.n_pages_requested = 0;

    result.n_pages_requested = n_pages_requested;
    for(PartitionRef iter = this->all_blocks.begin(); iter != this->all_blocks.end(); iter++){
      if(iter->tag == -1 && iter->size > result.max_free_partition_size){
        result.max_free_partition_size = iter->size;
        result.max_free_partition_address = iter->addr;
      }
    }
    return result;
  }
};

MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests) {

  Simulator sim(page_size);
  for(const auto & req : requests){
    if(req.tag < 0){
      sim.deallocate(-req.tag);
    } else{
      sim.allocate(req.tag, req.size);
    }
  }
  return sim.getStats();
}