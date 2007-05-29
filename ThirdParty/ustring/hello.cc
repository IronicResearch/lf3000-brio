#include <glibmm/ustring.h>
#include <stdio.h>

typedef Glib::ustring	CString;


int main(void) {

  CString str = "hello world";

  printf("%s\n", str.c_str());

}
