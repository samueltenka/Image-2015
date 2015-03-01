#include <fstream>


struct RGB {
   double R,G,B;
};
struct HW {
   int height, width;
};


static long fgetnum(std::FILE* file, int num_bytes) {
   /*assumes least significant byte comes first*/
   long long val = 0;
   long long power = 1;
   for(int i=0; i<num_bytes; ++i) {
      val += power * std::fgetc(file);
      power <<= 8;
   }
   return val;
}
static void fputnum(std::FILE* file, long long val, int num_bytes) {
   /*writes least significant byte first*/
   for(int i=0; i<num_bytes; ++i) {
      std::fputc(val & 0xff, file);
      val >>= 8;
   }
}


class Bitmap {
   RGB** data;
   HW dims;
   void allocate(HW dims) {
      data = new RGB*[dims.width];
      for(int r=0; r<dims.height; ++r) {
         data[r] = new RGB[dims.height];
      }
   }
   void deallocate() {
      for(int r=0; r<dims.height; ++r) {
         delete[] data[r];
      }
      delete[] data;
      data = NULL;
      dims = {0, 0};
   }
   int row_size() {                     // so 1--> 3--> 4
      /*bitmap rows are padded to be    //    2--> 6--> 8
        multiple of 4 bytes*/           //    3--> 9-->12
      int n = 3*dims.width+3;   //    4-->12-->12, etc.
      return n-(n%4);
   }
public:
   Bitmap() {}
   ~Bitmap() {
      deallocate();
   }
   void read_from(const char* filename) {
      deallocate();

      std::FILE* bmp = std::fopen(filename, "r");
      std::fseek(bmp, 18, SEEK_SET);
      dims.width = fgetnum(bmp, 4);
      dims.height = fgetnum(bmp, 4);
      allocate(dims);

      std::fseek(bmp, 18+36, SEEK_SET); // seeks from file beginning
      for(int r=0; r<dims.height; ++r) {
         for(int c=0; c<dims.width; ++c) {
            data[r][c].B = (double) std::fgetc(bmp);
            data[r][c].G = (double) std::fgetc(bmp);
            data[r][c].R = (double) std::fgetc(bmp);
         }
         for(int c3=3*dims.width; c3<row_size(); ++c3) { // padding
            std::fgetc(bmp);
         }
      }      
      std::fclose(bmp);
   }
   void write_to(const char* filename) {
      std::FILE* bmp = std::fopen(filename, "w");
      std::fputs("BM", bmp);
      int size = 18+36 + row_size()*dims.height;
      int header[14][2] = {{size, 4}, {0, 4}, {18+36, 4}, {40, 4}, // header size info
                           {dims.width, 4}, {dims.height, 4}, // bitmap dims
                           {1, 2}, {24, 2}, {0, 4}, // compression info
                           {3*row_size()*dims.height, 4}, {2835, 4}, {2835, 4}, // resolution
                           {0, 4}, {0, 4}}; // (unused) color table info
      for(int i=0; i<14; ++i) {
         fputnum(bmp, header[i][0], header[i][1]);
      }

      int G = 0;
      for(int r=0; r<dims.height; ++r) {
         for(int c=0; c<dims.width; ++c) {
            std::fputc((int) data[r][c].B, bmp);
            std::fputc((int) data[r][c].G, bmp);
            std::fputc((int) data[r][c].R, bmp);
         }
         for(int c3=3*dims.width; c3<row_size(); ++c3) { // padding
            std::fputc(0, bmp);
         }
      }
      std::fclose(bmp);
   }
};
