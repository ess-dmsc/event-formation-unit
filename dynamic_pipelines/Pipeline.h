

class Pipeline {
public:
  Pipeline(const char *name);
  int Rx(int);

private:
  char *mName;
  void *mHandle;
  int (*mRx)(int);
};
