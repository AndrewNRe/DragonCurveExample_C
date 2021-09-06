#define DRAGONCURVE_TEXTURE_MEMORY_OVERWRITE_SAFETY_ON 1 //Turn this on to go slower or off to go faster but might write to invalid memory depending on how many iterations you run for.

//In my experience, an array that's sized to store about 1 million bytes (roughly 1000 kilobytes) has worked well for me with around 20 iterations on the dragon curve.
//Use that as a baseline as I was going to write some allocation function to make it easier on you, the programmer, but I couldn't figure out the math for how the allocation
//size was to be set. If you're really good at math, the allocation sizes for the first 4 iterations are as follows.
//1st iteration = 1 required byte or bit
//2nd iteration = 3 required bytes or bits
//3rd iteration = 7 required bytes or bits
//4th iteration = 15 required bytes or bits
//It would appear that 2 * iteration + 1 works for the first three, but fails on the fourth. I tried other things like multiplying by a larger number and subtracting but it all failed :/
//As for the texture it's straightforward. Just allocate the dimensions you want ie 1920 * 1080.

unsigned int GenerateDragonCurveDirectionArray(bool* CurveDirectionArray, unsigned int DesiredIterationCount)
{
    //NOTE: You can easily just use each individual bit of the 8 bit integer to store 8 curve values instead of 1 for data size reduction. I am currently unsure due to naiveity if you'd
    //get a speedup by doing so but I imagine that you would since bit operations don't take long at all to my knowledge.
    //I am purposely writing the code this way for ease of understanding and not for speed.
    //To understand what R and reverse mean in the loops, I recomend you check this out https://en.wikipedia.org/wiki/Dragon_curve#:~:text=The%20folding%20patterns,letters%20swapped.%5B1%5D
    
    unsigned int CurveArrayIndex = 0;
    
    for(unsigned int d = 0;
        d < DesiredIterationCount;
        d++)
    {
        {//Add an R
            CurveDirectionArray[CurveArrayIndex] = true;
            CurveArrayIndex++;
        }//End of adding an R
        unsigned int A = CurveArrayIndex - 1;
        for(int i = A - 1, z = 0; 
            z < A;
            i--, z++)
        {//Reverse
            CurveDirectionArray[CurveArrayIndex + i] = !CurveDirectionArray[z];
        }
        CurveArrayIndex += A;
    }
    
    return CurveArrayIndex;
}

void DrawDragonCurve(unsigned int* Texture, unsigned int Width, unsigned int Height,
                     unsigned int FillColor, unsigned int PixelsPerLine, //PixelsPerLine is pretty much scale, FillColor is your color where I assume you have 32 bits per pixel in the texture.
                     int XStart, int YStart, //Where in the texture you want the fractal to originate from. I usually do Width/2 and Height/2.
                     int XStartDirection, int YStartDirection, //To explain this, you are just setting the direction where you want to "move" when drawing the first line.
                     //So -1 XStartDirection and 0 YStartDirection will move left, 1 and 0 will move right. 0 and 1 up, 0 and -1 down. This does change the produced image!
                     //One thing the inquisitive person might notice is that this routine, after the start, won't move diagonally. Try adding this if you want to once you as it is possible.
                     bool* CurveDirectionArray, unsigned int CurveArrayIndex)
{
    unsigned int Size = Width * Height;
    int X = XStart; int Y = YStart;
    int XIncrement = XStartDirection; int YIncrement = YStartDirection;
    
    for(unsigned int f = 0;
        f <= CurveArrayIndex;
        f++)
    {
        
        for(unsigned int p = 0;
            p < PixelsPerLine;
            p++,
            Y += YIncrement,
            X += XIncrement)
        {//Fill pixel routine
            int i = X + (Y * Width);
#if DRAGONCURVE_TEXTURE_MEMORY_OVERWRITE_SAFETY_ON
            if(i >= 0 && i < Size) 
#endif
                Texture[i] = FillColor;
        }//End fill pixel routine
        
        if(XIncrement)
        {
            if(XIncrement == 1)
            {
                YIncrement = (CurveDirectionArray[f]) ? -1 : 1;
            }
            else
            {
                YIncrement = (CurveDirectionArray[f]) ? 1 : -1;
            }
            XIncrement = 0;
        }
        else
        {
            if(YIncrement == 1)
            {
                XIncrement = (CurveDirectionArray[f]) ? 1 : -1;
            }
            else
            {
                XIncrement = (CurveDirectionArray[f]) ? -1 : 1;
            }
            YIncrement = 0;
        }
        
    }
    
}

#include <stdlib.h>
void ExampleCurveDraw()
{
    unsigned int DesiredIterationCount = 10;
    bool CurveDirectionArray[1024]; //NOTE: Only works with 10 or lower iterations.
    unsigned int CurveArrayIndex = GenerateDragonCurveDirectionArray(CurveDirectionArray, DesiredIterationCount);
    unsigned int Width = 1920;
    unsigned int Height = 1080;
    unsigned int WH = Width*Height;
    unsigned int* Texture = (unsigned int*)malloc(WH*sizeof(unsigned int));
    for(unsigned int i = 0;
        i < WH;
        i++)
    { Texture[i] = 0; } //Make background black.
    DrawDragonCurve(Texture, Width, Height,
                    0xFFE208AA, //Purple color using directx12 so just change this to whatever color format your renderer or end image uses.
                    4, //4 Pixels per drawn line
                    Width/2, Height/2, //Start in center of texture
                    1, -1, //Move diagonally at startup.
                    CurveDirectionArray, CurveArrayIndex);
    //Draw to the screen using whatever you want or save offline.
    free(Texture);
}