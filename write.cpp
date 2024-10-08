#include <iostream>
#include <fstream>
#include <vector>

#include "fpdfview.h"
#include "fpdf_edit.h"
#include "fpdf_save.h"
#include "ttc2ttf/ttc2ttf.h"

double GetTextWidth(FPDF_DOCUMENT document, FPDF_PAGE page, FPDF_FONT font, const wchar_t* text, double font_size)
{
    FPDF_PAGEOBJECT text_object = FPDFPageObj_CreateTextObj(document, font, font_size);
    if (!text_object)
        return 0;

    FPDF_WIDESTRING wide_text = (FPDF_WIDESTRING)text;
    FPDFText_SetText(text_object, wide_text);

    float left, bottom, right, top;
    if (!FPDFPageObj_GetBounds(text_object, &left, &bottom, &right, &top))
    {
        FPDFPageObj_Destroy(text_object);
        return 0;
    }

    FPDFPageObj_Destroy(text_object);

    return right - left;
}

FPDF_FONT LoadJapaneseFont(FPDF_DOCUMENT document, const char* font_path, int font_index = -1)
{
    std::vector<char> input;
    if (!file_read_all(font_path, input))
    {
        std::cerr << "Failed to open font file: " << font_path << std::endl;
        return nullptr;
    }

    std::vector<char> output;
    if (font_index >= 0)
    {
        TTC2TTF_RET ret = ttc2ttf_data_from_data(output, input, font_index);
        if (ret != TTC2TTF_RET_NO_ERROR)
            return nullptr;
        input.clear();
    }
    else
    {
        output = std::move(input);
    }

    FPDF_FONT font = FPDFText_LoadFont(document, reinterpret_cast<const uint8_t *>(output.data()), output.size(), FPDF_FONT_TRUETYPE, false);
    if (!font)
        std::cerr << "Failed to load font from file: " << font_path << std::endl;

    return font;
}

static inline float mm_to_points(float mm)
{
    return mm * (72.0f / 25.4f);
}

int main(void)
{
    // Initialize PDFium
    FPDF_LIBRARY_CONFIG config = { 2 };
    FPDF_InitLibraryWithConfig(&config);
    if (FPDF_GetLastError())
        return 1;

    // A4 paper size (210mm x 297mm) in points
    float width = mm_to_points(210), height = mm_to_points(297);

    // Create a PDF document
    FPDF_DOCUMENT document = FPDF_CreateNewDocument();
    if (!document)
    {
        std::cerr << "FPDF_CreateNewDocument failed" << std::endl;
        return -1;
    }

    // Create an A4 page
    FPDF_PAGE page = FPDFPage_New(document, 0, width, height);
    if (!page)
    {
        std::cerr << "FPDFPage_New failed" << std::endl;
        FPDF_CloseDocument(document);
        return -1;
    }

    // Load a font
    //FPDF_FONT font = FPDFText_LoadStandardFont(document, "Arial");
    //FPDF_FONT font = LoadJapaneseFont(document, "C:\\USERS\\KATAHIROMZ\\APPDATA\\LOCAL\\MICROSOFT\\WINDOWS\\FONTS\\NOTOSERIFJP-REGULAR.TTF");
    FPDF_FONT font = LoadJapaneseFont(document, "C:\\Windows\\Fonts\\msgothic.ttc", 2);
    if (!font)
    {
        std::cerr << "Loading font failed" << std::endl;
        FPDF_ClosePage(page);
        FPDF_CloseDocument(document);
        return -1;
    }

    const wchar_t* text = L"ABCabc123";

    // The size and position of text
    float font_size = 50;
    float text_width =  GetTextWidth(document, page, font, text, font_size);
    std::cerr << text_width << std::endl;
    float text_height = font_size;

    // Centering text
    float x = (width - text_width) / 2;
    float y = (height - text_height) / 2;

    // Draw text
    FPDF_PAGEOBJECT text_object = FPDFPageObj_CreateTextObj(document, font, font_size);
    FPDFText_SetText(text_object, (FPDF_WIDESTRING)text);
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, x, y);
    FPDFPage_InsertObject(page, text_object);
    FPDFPage_GenerateContent(page);

    // Save PDF as a file
    FILE *fp = fopen("output.pdf", "wb");
    fclose(fp);
    FPDF_FILEWRITE file_write = { 1, [](struct FPDF_FILEWRITE_* pThis, const void* pData, unsigned long size) -> int {
        FILE *fp = fopen("output.pdf", "ab");
        fwrite(pData, size, 1, fp);
        fclose(fp);
        return 1;
    }};
    FPDF_SaveAsCopy(document, &file_write, FPDF_NO_INCREMENTAL);

    // Clean up
    FPDF_ClosePage(page);
    FPDF_CloseDocument(document);
    FPDF_DestroyLibrary();

    return 0;
}
