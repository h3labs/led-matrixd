#ifndef __OPEN_CLOSE_SIGN_H__
#define __OPEN_CLOSE_SIGN_H__

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>

#include "file-status-notification.h"

namespace ledMatrixD {
  class OpenCloseSign : public FileStatusNotifee { 
    rgb_matrix::Canvas* canvas;
    rgb_matrix::ThreadedCanvasManipulator* image_gen;
    FileCreatedStatusObserver* observer;
    std::string isOpenFileName;
    bool isOpen;
    public:
    OpenCloseSign();
    void run();
    void notify(std::string fileName, int event);
    private:
    void showSign(bool first, bool newIsOpen);
  };
}
#endif
