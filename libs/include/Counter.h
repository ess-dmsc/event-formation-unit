/** Copyright (C) 2016 European Spallation Source */

#include <algorithm>
#include <numeric>
#include <vector>

template <class T> class Counter {

private:
  std::vector<T> counts;

public:
  void add(T t); /**< add element to template vector*/

  T avg(void); /**< calculate 'average' of vector */

  T max(void); /**< return max element of vector */

  T min(void); /**< return min element of vector */

  bool empty(void); /**< check if vector is empty */
};

template <class T> void Counter<T>::add(T elem) { counts.push_back(elem); }

template <class T> T Counter<T>::max(void) {
  return *std::max_element(counts.begin(), counts.end());
}

template <class T> T Counter<T>::min(void) {
  return *std::min_element(counts.begin(), counts.end());
}

template <class T> bool Counter<T>::empty(void) { return counts.empty(); }

template <class T> T Counter<T>::avg(void) {
  return std::accumulate(counts.begin(), counts.end(), (T)0) / counts.size();
}
