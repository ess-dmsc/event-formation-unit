/** Copyright (C) 2016 European Spallation Source */

#include <algorithm>
#include <vector>

template <class T> class Counter {

private:
  std::vector<T> counts;

public:
  void add(T t);

  T average(void);

  T max(void);

  T min(void);

  bool empty(void);
};

template <class T> void Counter<T>::add(T elem) { counts.push_back(elem); }

template <class T> T Counter<T>::max(void) {
  return *std::max_element(counts.begin(), counts.end());
}

template <class T> T Counter<T>::min(void) {
  return *std::min_element(counts.begin(), counts.end());
}

template <class T> bool Counter<T>::empty(void) { return counts.empty(); }
