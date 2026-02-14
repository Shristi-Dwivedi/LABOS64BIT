#include "pic.h"

void timer_handler(){
    pic_send_eoi(0);
}