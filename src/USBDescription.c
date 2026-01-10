#include <usb_names.h>

#define MANUFACTURER_NAME    {'C','r','a','z','y','S','o','a','p'}
#define MANUFACTURER_NAME_LEN 9
#define PRODUCT_NAME    {'P','o','l','y','G','r','a','f','t'}
#define PRODUCT_NAME_LEN 9

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME
};

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME
};