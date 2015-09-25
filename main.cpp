#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <zlib.h>
#include <string>
#include <algorithm>
#include <cstring>
#include<fstream>

/*! \brief error message buffer length */
const int kPrintBuffer = 1 << 12;


inline void HandleCheckError(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(-1);
}

/*!\brief same as assert, but this is intended to be used as message for user*/
inline void Check(bool exp, const char *fmt, ...) {
  if (!exp) {
    std::string msg(kPrintBuffer, '\0');
    va_list args;
    va_start(args, fmt);
    vsnprintf(&msg[0], kPrintBuffer, fmt, args);
    va_end(args);
    HandleCheckError(msg.c_str());
  }
}

/*! \brief replace fopen, report error when the file open fails */
inline std::FILE *FopenCheck(const char *fname, const char *flag) {
  std::FILE *fp = fopen64(fname, flag);
  Check(fp != NULL, "can not open file \"%s\"\n", fname);
  return fp;
}

/*! \brief implementation of file i/o stream */
class StdFile {
 public:
  /*! \brief constructor */
  StdFile(const char *fname, const char *mode) {
    Open(fname, mode);
  }
  StdFile() {}
  virtual ~StdFile(void) {
    this->Close();
  }
  virtual void Open(const char *fname, const char *mode) {
    fp_ = FopenCheck(fname, mode);
    fseek(fp_, 0L, SEEK_END);
    sz_ = ftell(fp_);
    fseek(fp_, 0L, SEEK_SET);
  }
  virtual size_t Read(void *ptr, size_t size) {
    return fread(ptr, size, 1, fp_);
  }
  virtual void Write(const void *ptr, size_t size) {
    fwrite(ptr, size, 1, fp_);
  }
  virtual void Seek(size_t pos) {
    fseek(fp_, pos, SEEK_SET);
  }
  virtual size_t Tell(void) {
    return static_cast<size_t>(ftell(fp_));
  }
  inline void Close(void) {
    if (fp_ != NULL){
      fclose(fp_); fp_ = NULL;
    }
  }
  inline size_t Size() {
    return sz_;
  }
 private:
  FILE *fp_;
  size_t sz_;
}; // class StdFile



/*! \brief Basic page class */
class BinaryPage {
 public:
  /*! \brief page size 64 MB */
  static const size_t kPageSize = 64 << 18;
 public:
  /*! \brief memory data object */
  struct Obj{
    /*! \brief pointer to the data*/
    void  *dptr;
    /*! \brief size */
    size_t sz;
    Obj(void * dptr, size_t sz) : dptr(dptr), sz(sz){}
  };
 public:
  /*! \brief constructor of page */
  BinaryPage(void)  {
    data_ = new int[kPageSize];
    Check(data_ != NULL, "fail to allocate page, out of space");
    this->Clear();
  };
  ~BinaryPage() {
    if (data_) delete [] data_;
  }
  /*!
   * \brief load one page form instream
   * \return true if loading is successful
   */
  inline bool Load(IStream &fi) {
    return fi.Read(&data_[0], sizeof(int)*kPageSize) !=0;
  }
  /*! \brief save one page into outstream */
  inline void Save(IStream &fo) {
    fo.Write(&data_[0], sizeof(int)*kPageSize);
  }
  /*! \return number of elements */
  inline int Size(void){
    return data_[0];
  }
  /*! \brief Push one binary object into page
   *  \param fname file name of obj need to be pushed into
   *  \return false or true to push into
   */
  inline bool Push(const Obj &dat) {
    if(this->FreeBytes() < dat.sz + sizeof(int)) return false;	// check if page can hold img + img_size
    data_[ Size() + 2 ] = data_[ Size() + 1 ] + dat.sz;		// save img_size
    memcpy(this->offset(data_[ Size() + 2 ]), dat.dptr, dat.sz);// save img	
    ++ data_[0];
    return true;
  }
  /*! \brief Clear the page */
  inline void Clear(void) {
    memset(&data_[0], 0, sizeof(int) * kPageSize);
  }
  /*!
   * \brief Get one binary object from page
   *  \param r r th obj in the page
   */
  inline Obj operator[](int r) {
    Assert(r < Size(), "index excceed bound");
    return Obj(this->offset(data_[ r + 2 ]),  data_[ r + 2 ] - data_[ r + 1 ]);
  }
 private:
  /*! \return number of elements */
  inline size_t FreeBytes(void) {
    return (kPageSize - (Size() + 2)) * sizeof(int) - data_[ Size() + 1 ];
  }
  inline void* offset(int pos) {
    return (char*)(&data_[0]) + (kPageSize*sizeof(int) - pos);
  }
 private:
  //int data_[ kPageSize ];
  int *data_;
};  // class BinaryPage


int main(void)
{
  
  StdFile reader("test.rec", "rb");
  std::cout << reader.Size() << " " << reader.Size()/1001.0 << std::endl;
  std::vector<int> buf( 100, 0 );
  
  reader.Read(&buf[0], 100);
  for ( int i = 0; i < 100; ++i ) {
    std::cout << reinterpret_cast<int>(buf[i]) << std::endl;
  }
  
  /*
    FILE* fp_;
    fp_ = fopen("test.rec", "wb");
    for (unsigned long long j = 0; j < 1024; ++j){
        //Some calculations to fill a[]
        fread(a, 1, size*sizeof(unsigned long long), fp_);
    }
    fclose(fp_);
    */
/*
    const size_t kPageSize = 64 << 18;
    int *data_ = new int[kPageSize];
    
    FILE * pFile;
    char buffer [100];
    int readSize = 0;
    int iter = 0;

    pFile = fopen ("test.rec" , "rb");
    if (pFile == NULL) perror ("Error opening file");
    else
    {
    //while ( ! feof (pFile) )
    //{
      while ((readSize = fread(data_, kPageSize, 1, pFile)) !=0 ) {
	std::cout << readSize << " ,iter " << iter << std::endl;
      for ( int i = 0; i < kPageSize; ++i ) {
      	std::cout << data_[i] << std::endl;
      }
      //if ( fgets (buffer , 100 , pFile) == NULL ) break;
      //fputs (buffer , stdout);
	++iter;
    }
    fclose (pFile);
    }
*/
  
    return 0;
}