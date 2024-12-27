/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Cat Shen

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    No

- Did you do any optional enhancements? If so, please explain:
    Yes
    I added Process 11, which will allow you to layer two images on top of each other 
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
#include <algorithm>
#include <string>

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
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates vignette gradient
            double dist_y = 1 - pow(2*abs(.5 - static_cast<double>(y)/static_cast<double>(file_height-1)),1.5);
            double dist_x = 1 - pow(2*abs(.5 - static_cast<double>(x)/static_cast<double>(file_width-1)),1.5);
            
            // Applies the gradient
            rgb.red = static_cast<int>(round(rgb.red * dist_y * dist_x));
            rgb.green = static_cast<int>(round(rgb.green * dist_y * dist_x));
            rgb.blue = static_cast<int>(round(rgb.blue * dist_y * dist_x));
            image_file[y][x] = rgb;
        }
    }
    cout << "Executed Process 01: Add Vignette" << endl;
    return image_file;
}

vector<vector<Pixel>> process_02 (vector<vector<Pixel>> image_file, double scaling)
/**
 * Makes a high contrast version of the image
 * If the average RGB value of the image is 
 * greater than 170, colors will be pushed brighter
 * and vice versa for lower than 90
 * @param image_file The image file to be editted
 * @param scaling Strength of the effect on the image
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates average brights and sets contrast levels
            double average = static_cast<double>(rgb.red + rgb.green + rgb.blue)/3;
            
            // Increases contrast of the image
            if (average > 170)
            {
                rgb.red = static_cast<int>(round(255 - (255 - rgb.red)*scaling));
                rgb.green = static_cast<int>(round(255 - (255 - rgb.green)*scaling));
                rgb.blue = static_cast<int>(round(255 - (255 - rgb.blue)*scaling));
            } else if (average < 90)
            {
                rgb.red = static_cast<int>(round(rgb.red*scaling));
                rgb.green = static_cast<int>(round(rgb.green*scaling));
                rgb.blue = static_cast<int>(round(rgb.blue*scaling));
            }
            // Applies the changes to the image file
            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 02: Claredon by factor of " << scaling << endl;
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
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
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
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Creates a new image that is rotated
    vector<vector<Pixel>> rotated_image (file_width, 
                                        vector<Pixel> (file_height)); 

    // Loops through the pixels in image
    // Looping through the dimensions of the
    // new image and copying the old image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];
            rotated_image[x][file_height-y-1] = rgb;

        }
    }
    cout << "Executed Process 04: Rotate 90 Degrees" << endl;
    return rotated_image;
}


vector<vector<Pixel>> process_05 (vector<vector<Pixel>> image_file, int turns)
/**
 * Rotates the image multiple times
 * @param image_file The image file to be editted
 * @param turns The number of times the image will be rotated
 */
{

    cout << "Rotating 90 degrees " << turns << " times" << endl;
    vector<vector<Pixel>> rotated_image  = image_file;
    int num_turns = turns;

    // If the image is being turned 0 times, or 360 times, we will return the 
    // original image without turning
    if (turns % 4 == 0 || turns == 0) 
    {
        return rotated_image;
    } 

    // This will count down turns and call process 04 for each count down
    while (turns != 0)
    {
        // Converts counterclockwise turns to clockwise turns
        while (turns < 0)
        {
            turns = turns + 4;
        }
        // Reduces number of turns for huge numbers
        while (turns >= 5)
        {
            turns = turns - 4;
        }
        // Applies the number of turns after reducing to minimum
        // number of clockwise turns
        if (turns < 5)
        {
            rotated_image  = process_04(rotated_image);
            turns--;
        }
        
    }
    cout << "Executed Process 05: Rotated 90 Degrees " << num_turns << " times" << endl;
    return rotated_image;
}

vector<vector<Pixel>> process_06 (vector<vector<Pixel>> image_file, float scale_x, float scale_y)
/**
 * Scales the image larger or smaller
 * @param image_file Image that will be scaled
 * @param scale_x Amount to scale the image by on the x axis
 * @param scale_y Amount to scale the image by on the y axis
 */
{
    // Prevents dividing by zero
    if (scale_x == 0 || scale_y == 0)
    {
        cout << "Cannot scale by 0. 0 value will default to 1." << endl;
        if (scale_x == 0)
        {
            scale_x = 1;
        }
        if (scale_y == 0)
        {
            scale_y = 1;
        }
    }

    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();
    // Creates a new vector that is the size of the scaled image
    int scaled_height = static_cast<int>(round(file_height * scale_y));
    int scaled_width = static_cast<int>(round(file_width * scale_x));
    vector<vector<Pixel>> scaled_image (scaled_height, 
                                        vector<Pixel> (scaled_width)); 

    // Loops over the newly created blank image
    for (int y = 0; y < scaled_height; y++)
    {
        for (int x = 0; x < scaled_width; x++)
        {
            // Estimates the closest old pixel and copies 
            // to the resized version
            int descaled_height = static_cast<int>(round(y/scale_y));
            int descaled_width = static_cast<int>(round(x/scale_x));

            // Preventing this from rounding up over out of bounds
            while (descaled_height > file_height-1){
                descaled_height--;
            }
            while (descaled_width > file_width-1){
                descaled_width--;
            }
            Pixel rgb = image_file[descaled_height][descaled_width];
            scaled_image[y][x] = rgb;

        }
    }    
    cout << "Executed Process 06: Scaled image " << scale_x << " by " << scale_y << endl;
    return scaled_image;
}

vector<vector<Pixel>> process_07 (vector<vector<Pixel>> image_file)
/**
 * Changes the high contrast black and white
 * @param image_file The image file to be editted
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates average greyscale value
            int grey_val = (rgb.red + rgb.green + rgb.blue)/3;

            // Sets value based on average
            if (grey_val >= 255/2)
            {
                rgb.red = 255;
                rgb.green = 255;
                rgb.blue = 255;
            } else 
            {
                rgb.red = 0;
                rgb.green = 0;
                rgb.blue = 0;
            }
            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 07: B&W" << endl;
    return image_file;
}

vector<vector<Pixel>> process_08 (vector<vector<Pixel>> image_file, double scaling)
/**
 * Lightens image by the scaling factor
 * @param image_file The image file to be editted
 * @param scaling Strength of the effect on the image
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates lightened value with scaling factor
            rgb.red = static_cast<int>(round(255-(255 - rgb.red)*scaling));
            rgb.green = static_cast<int>(round(255-(255 - rgb.green)*scaling));
            rgb.blue = static_cast<int>(round(255-(255 - rgb.blue)*scaling));

            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 08: Lightened by factor of " << scaling << endl;
    return image_file;
}

vector<vector<Pixel>> process_09 (vector<vector<Pixel>> image_file, double scaling)
/**
 * Darkens image by the scaling factor
 * @param image_file The image file to be editted
 * @param scaling Strength of the effect on the image
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates darkened value with scaling factor
            rgb.red = static_cast<int>(round(rgb.red*scaling));
            rgb.green = static_cast<int>(round(rgb.green*scaling));
            rgb.blue = static_cast<int>(round(rgb.blue*scaling));

            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 09: Darken by factor of " << scaling << endl;
    return image_file;
}

vector<vector<Pixel>> process_10 (vector<vector<Pixel>> image_file)
/**
 * Extreme contrast, extreme satruation
 * Black, white, RGB
 * @param image_file The image file to be editted
 */
{
    // Get the size of the image
    int file_height = image_file.size();
    int file_width = image_file[0].size();

    // Loops through the pixels in image
    for (int y = 0; y < file_height; y++)
    {
        for (int x = 0; x < file_width; x++)
        {
            Pixel rgb = image_file[y][x];

            // Calculates max values and combined values
            vector <int> pixel_vector = {rgb.red, rgb.green, rgb.blue};
            int max_color = distance(pixel_vector.begin(), 
                                    max_element(pixel_vector.begin(), pixel_vector.end()));
            int add_color = rgb.red + rgb.green + rgb.blue;

            // Sets B*W values for highest contrast areas 
            if (add_color >= 550)
            {
                rgb.red = 255;
                rgb.green = 255;
                rgb.blue = 255;
            } else if (add_color <= 150)
            {
                rgb.red = 0;
                rgb.green = 0;
                rgb.blue = 0;                
            } else {
                // Sets RGB values for medium areas
                switch (max_color) {
                    case 0:
                        rgb.red = 255;
                        rgb.green = 0;
                        rgb.blue = 0;
                        break;  
                    case 1:
                        rgb.red = 0;
                        rgb.green = 255;
                        rgb.blue = 0;
                        break;    
                    case 2:
                        rgb.red = 0;
                        rgb.green = 0;
                        rgb.blue = 255;  
                        break;  
                }
            }

            image_file[y][x] = rgb;

        }
    }
    cout << "Executed Process 10: Black, White and RGB" << endl;
    return image_file;
}

vector<vector<Pixel>> process_11 (vector<vector<Pixel>> image_file, vector<vector<Pixel>> layer_image, double scaling)
{
/**
 * Layers one image on top of the other, using the scaling argument to blend the two
 * Second image will be centered on the original image
 * @param image_file The image file on the bottom
 * @param image_file The image file layers on top
 * @param scaling The transparency of the top image
 */
    // Get the size of the images
    int file_height = image_file.size();
    int file_width = image_file[0].size();
    int layer_height = layer_image.size();
    int layer_width = layer_image[0].size();
    int y_difference = abs(round((layer_height-file_height)/2));
    int x_difference = abs(round((layer_width-file_width)/2));

    // Setting up image placement
    // Layered image will be centered on bottom image

    // Crops the layer image to the same size as original image
    // if it is larger. Removes pixels evenly from either side.
    // Crops down height
    
    if (file_height < layer_height)
    {
        for (int n = 0; n < y_difference; n++)
        {
            layer_image.erase(layer_image.begin());
            layer_image.erase(layer_image.end()-1);
        }
        // Reassigns height size and recalcs height difference
        layer_height = layer_image.size();
        y_difference = round((layer_height-file_height)/2);
    }
    // Crops down width
    if (file_width < layer_width)
    {
        for (int y = 0; y < layer_height; y++)
        {
            for (int n = 0; n < x_difference; n++)
            {
                layer_image[y].erase(layer_image[y].begin());
                layer_image[y].erase(layer_image[y].end()-1);
            }
        }
        // Reassigns width and recalcs width difference
        layer_width = layer_image[0].size();
        x_difference = round((layer_width-file_width)/2);
    }
    
    // Adds the nwe image on top of the old image using the scaling parameter
    // to calculate transparency. Centers the layered image on top
    for (int y = 0; y < layer_height; y++)
    {
        for (int x = 0; x < layer_width; x++)
        {
            Pixel image_rgb = image_file[y+y_difference][x+x_difference];
            Pixel layer_rgb = layer_image[y][x];

            image_rgb.red = image_rgb.red * (scaling) + layer_rgb.red * (1 - scaling);
            image_rgb.green = image_rgb.green * (scaling) + layer_rgb.green * (1 - scaling);
            image_rgb.blue = image_rgb.blue * (scaling) + layer_rgb.blue * (1 - scaling);

            image_file[y+y_difference][x+x_difference] = image_rgb;
            
        }
    }
    cout << "Executed Process 11: Layer Images with " << scaling << " Transparency" << endl;
    return image_file;
}


int main()
/* 
Provides a UI for the image processing application
Asks for a image path and then provides menu
*/
{
    // Initial variable set up
    string input_val;           // Checks inputs for initial file path entry
    string menu_val;            // Checks inputs for menu of imgage modifications
    string file_path;
    // Initial menu UI
    cout << endl << "----------------======----------------" << endl
    << "CSPB 1300 Image Processing Application" << endl
    << "----------------======----------------" << endl
    << "This application will take an input image file and make modifications to it" ;

    // Setting up a while loop to run the application repeatedly until 
    // exit is called
    while (input_val != "q" || input_val != "Q" )
    {

        cout << endl << " -- Enter q to exit" << endl << endl
        << "   Enter a file path: " << endl;

        // Input takes file path, loops back to top if invalid
        cin >> file_path;

        if (cin.fail())
        {
            // Validates input, clears errors if invalid
            cin.clear();
            cout << endl << "ERROR: Invalid input, please try again" << endl << endl;
            break;

        } else if (file_path == "q" || file_path == "Q") 
        {
            // Exits while loop and the application
            cout << "   Quitting... " << endl;
            input_val = "q";
            break;
        } else
        {
            // If path is a valid entry, will try to open the image file
            // Image with size 0 / unable to read will loop back to the menu
            try 
            {
                vector<vector<Pixel>> input_image = read_image(file_path);
                if (input_image.size() <= 0) {
                    cout << endl << "ERROR: Unable to read image, please try again" << endl
                    << "Please ensure your image is a .bmp file and the path is valid"<< endl << endl;
                    input_val = "c";
                } else 
                {
                    cout <<endl << "   Image read sucessfully" << endl << endl;
                    /*
                    Main Menu input
                    Provides various options to process images
                    */ 
                   while (menu_val != "q" || menu_val != "Q")
                   {
                    cout << endl << "------------------------" << endl
                    << "IMAGE PROCESSING OPTIONS" << endl
                    << "------------------------" << endl
                    << "   Current image: " << file_path << endl << endl
                    << "0) Change Image" << endl
                    << "1) Vignette" << endl 
                    << "2) Claredon" << endl
                    << "3) Greyscale" << endl 
                    << "4) Rotate 90 Degrees" << endl 
                    << "5) Rotate Multiple Times" << endl 
                    << "6) Scale" << endl
                    << "7) Black and White" << endl 
                    << "8) Lighten" << endl 
                    << "9) Darken" << endl 
                    << "10) Black, White, RGB" << endl 
                    << "11) Layer Images" << endl << endl
                    << " -- Enter q to exit" << endl;

                    cin >> menu_val; 

                    // Exits the application with a q
                    if (menu_val == "q" || menu_val == "Q" ){
                        cout << "   Quitting... " << endl;
                        return 0;
                    } else
                    {
                        // Will attempt to convert string memu entry into a int
                        // If it fails, it will be caught and returned to the menu
                        // On sucess, will generate an empty vector for the modified image
                        try
                        {
                            int menu = stoi(menu_val);
                            if (menu >= 0 && menu <= 11){

                                vector<vector<Pixel>> process_image;
                                int image_modified = 0;         // Tracks if image was sucessfully modified
                                double scaling;                 // Strength of effect to be applied
                                string old_path = file_path;    // Saves path before attempting file change

                                switch (menu) {
                                    case 0:
                                    // Option to select a new image
                                        cout << endl << "  Input new file path:" << endl;
                                        cin >> file_path;
                                        input_image = read_image(file_path);
                                        if (input_image.size() <= 0) {
                                            cout << endl << "ERROR: Unable to read image, please try again" << endl
                                            << "Please ensure your image is a .bmp file and the path is valid" << endl << endl;
                                            file_path = old_path;
                                            break;
                                        } else 
                                        {
                                            cout <<endl << "   Image read sucessfully" << endl << endl;
                                            break;
                                        }

                                    case 1:
                                    // Process 01 - Adds a Vignette
                                        cout << endl << "  Running: Process 01" << endl;
                                        process_image = process_01(input_image);
                                        image_modified = 1;
                                        break;

                                    case 2:
                                    // Process 2 - Claredon Effect
                                        cout << "   Strength of effect:" << endl
                                        << "Enter a value between 0 and 1 " << endl
                                        << "   0 ------------ .5 ------------ 1 " << endl
                                        << "strong ------------------------ weak" << endl;
                                        cin >> scaling;     // Strength of effect
                                        if (cin.fail() || scaling > 1 || scaling < 0)
                                        {
                                            cin.clear();
                                            cout << endl <<"ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else 
                                        {
                                        cout << endl << "  Running: Process 02" << endl;
                                        process_image = process_02(input_image, scaling);
                                        image_modified = 1;
                                        break;
                                        }

                                    case 3:
                                    // Process 3 - Greyscale Image
                                        cout << endl << "  Running: Process 03" << endl;
                                        process_image = process_03(input_image);
                                        image_modified = 1;
                                        break;

                                    case 4:
                                    // Process 4 - Rotate by 90 Degrees
                                        cout << endl << "  Running: Process 04" << endl;
                                        process_image = process_04(input_image);
                                        image_modified = 1;
                                        break;

                                    case 5:
                                    // Process 5 - Rotate by 90 Degrees multiple times
                                        double rot_input;      // Number of rotations
                                        cout << endl << "  Running: Process 05" << endl
                                        << "   Enter number of rotations: " << "(Integers only) " << endl;
                                        cin >> rot_input;
                                        if (cin.fail())
                                        {
                                            cin.clear();
                                            cin.ignore();
                                            cout << endl << "ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else 
                                        {
                                            int rotations = static_cast<int>(round(rot_input));
                                            process_image = process_05(input_image, rotations);
                                            image_modified = 1;
                                            break;
                                        }
                                        break;


                                    case 6:
                                    // Process 6 - Scale image on x and y axis
                                        double x_scale;     // Amount to scale x
                                        double y_scale;     // Amounts to scale y
                                        cout << endl << "  Running: Process 06" << endl
                                        << "   Enter x scale value and y scale value" << endl << "(Positive numbers) " << endl;
                                        cin >> x_scale >> y_scale;

                                        if (cin.fail())
                                        {
                                            cin.clear();
                                            cout << endl << "ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else 
                                        {
                                            process_image = process_06(input_image, x_scale, y_scale);
                                            image_modified = 1;
                                            break;
                                        }
                                        break;

                                    case 7:
                                    // Process 7 - Black and White Conversion
                                        cout << endl << "  Running: Process 07" << endl;
                                        process_image = process_07(input_image);
                                        image_modified = 1;
                                        break;

                                    case 8:
                                    // Process 8 - Lighten
                                        cout << "   Strength of effect:" << endl
                                        << "Enter a value between 0 and 1 " << endl
                                        << "   0 ------------ .5 ------------ 1 " << endl
                                        << "strong ------------------------ weak" << endl;
                                        cin >> scaling;     // Strength of effect

                                        if (cin.fail() || scaling > 1 || scaling < 0)
                                        {
                                            cin.clear();
                                            cout << endl <<"ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else 
                                        {
                                        cout << endl << "  Running: Process 08" << endl;
                                        process_image = process_08(input_image, scaling);
                                        image_modified = 1;
                                        break;
                                        }

                                    case 9:
                                    // Process 9 - Darken
                                        cout << "   Strength of effect:" << endl
                                        << "Enter a value between 0 and 1 " << endl
                                        << "   0 ------------ .5 ------------ 1 " << endl
                                        << "strong ------------------------ weak" << endl;
                                        cin >> scaling;     // Strength of effect

                                        if (cin.fail() || scaling > 1 || scaling < 0)
                                        {
                                            cin.clear();
                                            cout << endl <<"ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else 
                                        {
                                        cout << endl << "  Running: Process 09" << endl;
                                        process_image = process_09(input_image, scaling);
                                        image_modified = 1;
                                        break;
                                        }

                                    case 10:
                                    // Process 10 - Black, White, RGB
                                        cout << endl << "  Running: Process 10" << endl;
                                        process_image = process_10(input_image);
                                        image_modified = 1;
                                        break;
                                    
                                    case 11:
                                    // Process 11 - Layer two images
                                        string layer_path;
                                        cout << endl << "  Running: Process 11" << endl
                                        << endl << "  Input top layer image path:" << endl;
                                        cin >> layer_path;
                                        if (cin.fail())
                                        {
                                            cin.clear();
                                            cout << endl <<"ERROR: Invalid input" << endl;
                                            image_modified = 0;
                                            menu_val = "404";
                                            // Loops back to menu on invalid input
                                            break;
                                        } else {
                                            vector<vector<Pixel>> layer_image = read_image(layer_path);
                                            if (layer_image.size() <= 0) {
                                                cout << endl << "ERROR: Unable to read image, please try again" << endl
                                                << "Please ensure your image is a .bmp file and the path is valid" << endl << endl;
                                                break;
                                            } else 
                                            {
                                                cout <<endl << "   Image read sucessfully" << endl << endl;
                                                process_image = process_11(input_image, layer_image, .5);
                                                image_modified = 1;
                                                cout << endl << "Process 11 Complete, writing.." << endl;
                                        }
                                        }

                                }

                                // If image has been modified, it will ask for a file path
                                // to output the new image
                                // Returns to menu and discards changes on invalid entry or q
                                if (image_modified == 1)
                                {
                                    string output_path;
                                    cout << "  Enter image output path: " << endl
                                    << " -- Enter q to discard changes and return to menu" << endl;
                                    cin >> output_path;
                                    if (cin.fail() || output_path == "q" || output_path == "Q"){
                                        cin.clear();
                                        
                                    } else if (output_path == file_path) 
                                    {
                                        cout << "Cannot overwrite read file. Cancelling operation" << endl;
                                    }
                                    else
                                    {
                                        if (write_image(output_path, process_image))
                                        {
                                            cout << "Sucessful write: " << output_path << endl;
                                        } else 
                                        {
                                            cout << "Failed to output" << endl;
                                        }
                                    }  
                                }
                            }
                            else
                            // Error catcher for user input of integer <0 or > 10
                            {
                                cout << endl << "ERROR: Invalid menu option" << endl
                                << "Please try again" << endl << endl;   
                            }
                        } catch (exception& ex)
                        // catch for errors in image processing menu options
                        {
                            cout << endl << "ERROR: Invalid menu option" << endl
                            << "Please try again" << endl << endl;
                        }
                    }
                } 
            }
            // catch for errors in running the image processing
            } catch(exception& ex) {
                cout << endl << "ERROR: Unable to process image, please try again" << endl;
            } 
        } 
   }
}