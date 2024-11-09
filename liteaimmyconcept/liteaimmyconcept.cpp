#include <iostream>
#include <ncnn/net.h>
#include <ncnn/mat.h>
#include <Windows.h>
#include <vector>

// Screen capture thing
#include <wingdi.h>

ncnn::Net net;

bool LoadModel(const std::string& param_path, const std::string& bin_path) {
    if (net.load_param(param_path.c_str()) == 0 && net.load_model(bin_path.c_str()) == 0) {
        std::cout << "Model loaded successfully." << std::endl;
        return true;
    }
    std::cerr << "Failed to load model." << std::endl;
    return false;
}

//NCNN Mat
ncnn::Mat CaptureScreen(int x, int y, int width, int height) {
    //!! device context w/bitmap
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);

    // copy screen to memory device context
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, x, y, SRCCOPY);

	// store the screen capture in a buffer
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; // RGB
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<unsigned char> buffer(width * height * 3);
    GetDIBits(hMemoryDC, hBitmap, 0, height, buffer.data(), &bmi, DIB_RGB_COLORS);


    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return ncnn::Mat::from_pixels(buffer.data(), ncnn::Mat::PIXEL_RGB, width, height); // convert
}


void PerformInference(ncnn::Mat& in) {
    ncnn::Extractor ex = net.create_extractor();
	ex.input("input", in); // to be changed

    ncnn::Mat out;
    ex.extract("output", out); // to be changed

    for (int i = 0; i < out.h; i++) {
        const float* values = out.row(i);
        std::cout << "Object at [" << values[0] << ", " << values[1] << "] with confidence: " << values[4] << std::endl; // not even sure if this is right
    }
}

void MainLoop() {
    while (true) {
        // Check if 'Q' is pressed for inference
        if (GetAsyncKeyState('Q') & 0x8000) { // typing a bunch of bs
            int width = 640, height = 640; 
            ncnn::Mat screen_image = CaptureScreen(0, 0, width, height);

            PerformInference(screen_image);
        }

        // Delay to prevent high CPU usage
        Sleep(100);
    }
}

int main() {
	std::cout << "LiteAIMMyConcept" << std::endl;
	if (!LoadModel("modelto.ncnn.param", "model.ncnn.bin")) { // i cant test bc my model not working . . .
        return -1;
    }

    std::cout << "Press 'Q' to capture screen and run object detection." << std::endl;

    // Run the main loop
    MainLoop();

    return 0;
}
