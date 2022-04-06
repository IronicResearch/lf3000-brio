#include <glibmm/ustring.h>
#include <stdio.h>

typedef Glib::ustring	CString;


int main(void) {

  CString str = "Hello Brain's World";

  printf("%s\n", str.c_str());

}
