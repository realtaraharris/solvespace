// utility functions that are provided in the platform-independent code
#ifndef UTIL_H
#define UTIL_H

class utf8_iterator : std::iterator<std::forward_iterator_tag, char32_t> {
  const char *p, *n;
public:
  utf8_iterator(const char *p) : p(p), n(NULL) {}
  bool           operator==(const utf8_iterator &i) const { return p==i.p; }
  bool           operator!=(const utf8_iterator &i) const { return p!=i.p; }
  ptrdiff_t      operator- (const utf8_iterator &i) const { return p -i.p; }
  utf8_iterator& operator++()    { **this; p=n; n=NULL; return *this; }
  utf8_iterator  operator++(int) { utf8_iterator t(*this); operator++(); return t; }
  char32_t       operator*();
  const char*    ptr() const { return p; }
};

class ReadUTF8 {
  const std::string &str;
public:
  ReadUTF8(const std::string &str) : str(str) {}
  utf8_iterator begin() const { return utf8_iterator(&str[0]); }
  utf8_iterator end()   const { return utf8_iterator(&str[0] + str.length()); }
};

#define PI (3.1415926535897931)

int64_t GetMilliseconds();
void Message(const char *fmt, ...);
void MessageAndRun(std::function<void()> onDismiss, const char *fmt, ...);
void Error(const char *fmt, ...);

#endif // UTIL_H