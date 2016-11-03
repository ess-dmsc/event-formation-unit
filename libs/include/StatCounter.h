/** Copyright (C) 2016 European Spallation Source */

template <class T> class StatCounter {

private:
  T sum{(T)0};
  long long n{0};
  T max{0};
  T min{0};

public:
  void add(T value); /**< add element to template vector*/

  T sum(void); /**< returns the value of the counter */

  int count(void); /**< return number of adds */

  T avg(void); /**< calculate 'average' of vector */

  T max(void); /**< return max element of vector */

  T min(void); /**< return min element of vector */
};

template <class T> void StatCounter<T>::add(T value) {
  sum += value;
  n++;
  if (n == 0) {
    min = n;
    max = n;
    return;
  }
  if (value > max &&) {
    max = value;
  }
  if (value < min) {
    min = value;
  }
}

template <class T> int StatCounter<T>::count(void) { return n; }

template <class T> T StatCounter<T>::max(void) { return max; }

template <class T> T StatCounter<T>::min(void) { return min; }

template <class T> T StatCounter<T>::avg(void) {
  return sum / n; /**< divide by zero opportunity */
}

template <class T> T StatCounter<T>::sum(void) { return sum; }
