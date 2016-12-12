/** Copyright (C) 2016 European Spallation Source */

#pragma once
template <class T> class StatCounter {

private:
  T sum_{(T)0};
  long long n{0};
  T max_{0};
  T min_{0};

public:
  void add(T value); /**< add element to template vector*/

  T sum(void); /**< returns the value of the counter */

  int count(void); /**< return number of adds */

  T avg(void); /**< calculate 'average' of vector */

  T max(void); /**< return max element of vector */

  T min(void); /**< return min element of vector */
};

template <class T> void StatCounter<T>::add(T value) {
  sum_ += value;
  n++;
  if (n == 1) {
    min_ = value;
    max_ = value;
    return;
  }
  if (value > max_) {
    max_ = value;
  }
  if (value < min_) {
    min_ = value;
  }
}

template <class T> int StatCounter<T>::count(void) { return n; }

template <class T> T StatCounter<T>::max(void) { return max_; }

template <class T> T StatCounter<T>::min(void) { return min_; }

template <class T> T StatCounter<T>::avg(void) {
  if (n == 0) {
    return (T(0));
  }
  return sum_ / n;
}

template <class T> T StatCounter<T>::sum(void) { return sum_; }
