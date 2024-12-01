/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    <ANSWER>

- All project requirements fully met? (YES or NO):
    <ANSWER>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//
vector<vector<Pixel>> process_01 (vector<vector<Pixel>> image_file)
/**
 * Adds a vignette to the image
 * @param image_file The image file to be editted
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (double y = 0; y < file_height; y ++)
    {
        for (double x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates vignette gradient
            float dist_y = 1 - pow(2*abs(.5 - y/(file_height-1)),2);
            float dist_x = 1 - pow(2*abs(.5 - x/(file_width-1)),2);
            
            // Applies the gradient
            rgb.red = rgb.red * dist_y * dist_x;
            rgb.green = rgb.green * dist_y * dist_x;
            rgb.blue = rgb.blue * dist_y * dist_x;
            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 01: Add Vignette" << endl;

    return image_file;
}

vector<vector<Pixel>> process_02 (vector<vector<Pixel>> image_file)
/**
 * Makes a high contrast version of the image
 * If the average RGB value of the image is 
 * greater than 170, colors will be pushed brighter
 * and vice versa for lower than 90
 * @param image_file The image file to be editted
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (double y = 0; y < file_height; y ++)
    {
        for (double x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates average brights and sets contrast levels
            float average = (rgb.red + rgb.green + rgb.blue)/3;
            float scaling = .3;
            
            // Increases contrast of the image
            if (average >= 170)
            {
                rgb.red = 255 - (255 - rgb.red)*scaling;
                rgb.green = 255 - (255 - rgb.green)*scaling;
                rgb.blue = 255 - (255 - rgb.blue)*scaling;
            } else if (average < 90)
            {
                rgb.red = rgb.red*scaling;
                rgb.green = rgb.green*scaling;
                rgb.blue = rgb.blue*scaling;
            }

            // Applies the changes to the image file
            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 02: Add Contrast" << endl;

    return image_file;
}

vector<vector<Pixel>> process_03 (vector<vector<Pixel>> image_file)
/**
 * Changes the image to greyscale
 * @param image_file The image file to be editted
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (double y = 0; y < file_height; y ++)
    {
        for (double x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates average greyscale value
            int grey_val = (rgb.red + rgb.green + rgb.blue)/3;
            
            // Applies the gradient
            rgb.red = grey_val;
            rgb.green = grey_val;
            rgb.blue = grey_val;
            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 03: Greyscale" << endl;

    return image_file;
}

vector<vector<Pixel>> process_04 (vector<vector<Pixel>> image_file)
/**
 * Rotates the image by 90 degrees
 * @param image_file The image file to be editted
 */
{
    cout << "Starting rotation with square! " << endl;
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();
    cout << "Input Height: " << file_height << " Input Width: " << file_width << endl;

    // Creates a new image that is rotated
    vector<vector<Pixel>> image_rotate (file_width, 
                                        vector<Pixel> (file_height)); 

    // Loops through the pixels in image
    // Looping through the dimensions of the
    // new image and copying the old image
    for (double y = 0; y < file_height; y ++)
    {
        for (double x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];
            image_rotate[x][file_height -y] = rgb;

        }
    }
    cout << "Old File Height: " << file_height << " Old File Width: " << file_width << endl;
    cout << "New File Height: " << image_rotate.size() << " New File Width: " << image_rotate[0].size() << endl;
    cout << "Old Pixel (24, 165): " << image_file[124][165].red << "," << image_file[124][165].green << "," << image_file[124][165].blue << endl;
    cout << "New Pixel (24, 165): " << image_rotate[124][165].red << "," << image_rotate[124][165].green << "," << image_rotate[124][165].blue << endl;
    cout << "Executed Process 04: Rotate 90 Degrees" << endl;

    return image_rotate;
}

vector<vector<Pixel>> process_05 (vector<vector<Pixel>> image_file, int angle)
/**
 * Rotates the image multiple times
 * @param image_file The image file to be editted
 * @param angle The angle which the image will be rotated
 */
{
    //*/ Calculates how many times will rotate
    int turns = angle / 90;
    cout << "# of Turns: " << turns << endl;
    if (angle % 90 != 0)
    {
        cout << "Angle is not a multiple of 90, rounding..." 
        << endl;
    }

    //while (turns > 5)
    //    turns = turns-4; 

    //if (turns == 4) 
    //{
    //    return image_file;
    //} 

    vector<vector<Pixel>> image_rotate_01 = process_04(image_file);
    vector<vector<Pixel>> image_rotate_02 = process_04(image_rotate_01);
    vector<vector<Pixel>> image_rotate_03 = process_04(image_rotate_02);
    vector<vector<Pixel>> image_rotate_04 = process_04(image_rotate_03);

    cout << "Executed Process 05: Rotate Multiples of 90 Degrees" << endl;

    return image_rotate_01;

}

int main()
{


    // Reads in an image at a given path
    string file_path = "E:/courses/02_CSPB/24_Fall/CSPB_1300_CompSci/final_project/CSPB-1300-C-Final-Project/sample.bmp";
	vector<vector<Pixel>> input_image = read_image(file_path);

    // Process 1
    //vector<vector<Pixel>> process01_image = process_01(input_image);

    // Process 2
    //vector<vector<Pixel>> process02_image = process_02(input_image);

    //Process 3
    //vector<vector<Pixel>> process03_image = process_03(input_image);

    //Process 4
    //vector<vector<Pixel>> process04_image = process_04(input_image);

    //Process 5
    vector<vector<Pixel>> process05_image = process_05(input_image, 270);

    // Outputs if it was done sucessfully
    string output_path = "test_output.bmp";
    if (write_image(output_path, process05_image))
    {
        cout << "Sucessful write: " << output_path << endl;
    } else 
    {
        cout << "Failed to output" << endl;
    }
    return 0;

}