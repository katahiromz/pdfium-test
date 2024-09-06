#include <windows.h>
#include <windowsx.h>
#include <cstdio>

#include "fpdfview.h"
#include "SaveBitmapToFile.h"

void GetPDFPageSizeInPixels(FPDF_DOCUMENT doc, int page_index, double dpi_x, double dpi_y, int& width_pixels, int& height_pixels)
{
    double width_points, height_points;
    FPDF_GetPageSizeByIndex(doc, page_index, &width_points, &height_points);

    width_pixels = static_cast<int>(width_points * dpi_x / 72.0);  // 1 point = 1/72 inches
    height_pixels = static_cast<int>(height_points * dpi_y / 72.0);
}

HBITMAP Create24BppBitmap(HDC hDC, INT width, INT height)
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    LPVOID pvBits;
    return CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
}

int main(int argc, char **argv)
{
    // Initialize PDFium
    FPDF_LIBRARY_CONFIG config = { 2 };
    FPDF_InitLibraryWithConfig(&config);
    if (FPDF_GetLastError())
        return 1;

    FPDF_DOCUMENT doc = FPDF_LoadDocument("example.pdf", nullptr);
    if (!doc)
    {
        printf("Failed to load PDF document.\n");
        FPDF_DestroyLibrary();
        return -1;
    }

    double dpi = 10.0;

    int width_pixels, height_pixels;
    GetPDFPageSizeInPixels(doc, 0, dpi, dpi, width_pixels, height_pixels);

    printf("Page size: %d x %d pixels\n", width_pixels, height_pixels);

    FPDF_PAGE page = FPDF_LoadPage(doc, 0);
    if (!page)
    {
        printf("Failed to load PDF page.\n");
        FPDF_CloseDocument(doc);
        FPDF_DestroyLibrary();
        return -1;
    }

    HDC hDC = CreateCompatibleDC(NULL);
    HBITMAP hbm = Create24BppBitmap(hDC, width_pixels, height_pixels);
    HGDIOBJ hbmOld = SelectObject(hDC, hbm);
    RECT rc = { 0, 0, width_pixels, height_pixels };
    FillRect(hDC, &rc, GetStockBrush(WHITE_BRUSH));
    FPDF_RenderPage(hDC, page,
                          0, 0, width_pixels, height_pixels,
                          0, 0);
    SelectObject(hDC, hbmOld);
    DeleteDC(hDC);

    SaveBitmapToFile(TEXT("TEST.bmp"), hbm);
    DeleteObject(hbm);

    FPDF_ClosePage(page);
    FPDF_CloseDocument(doc);
    FPDF_DestroyLibrary();

    return 0;
}
