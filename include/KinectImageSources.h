#include <XnOpenNI.h>
#include <XnTypes.h>
#include <XnCppWrapper.h>

#include "cinder/ImageIo.h"

using namespace ci;

class ImageSourceKinectColor : public ImageSource
{
  public:
    ImageSourceKinectColor(xn::ImageMetaData &newData) : ImageSource()
    {
      _myMetaData.InitFrom(newData);
      _width = _myMetaData.XRes();
      _height = _myMetaData.YRes();
      mData = _myMetaData.WritableData();
      setSize( _width, _height );
      setColorModel( ImageIo::CM_RGB );
      setChannelOrder( ImageIo::RGB );
      setDataType( ImageIo::UINT8 );
    }
    ImageSourceKinectColor( uint8_t *buffer, int width, int height )
      : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
      setSize( _width, _height );
      setColorModel( ImageIo::CM_RGB );
      setChannelOrder( ImageIo::RGB );
      setDataType( ImageIo::UINT8 );
    }

    ~ImageSourceKinectColor()
    {
      // mData is actually a ref. It's released from the device.
      /*if( mData ) {
        delete[] mData;
        mData = NULL;
        }*/
    }

    virtual void load( ImageTargetRef target )
    {
      ImageSource::RowFunc func = setupRowFunc( target );

      for( uint32_t row	 = 0; row < _height; ++row )
        ((*this).*func)( target, row, mData + row * _width * 3 );
    }

  protected:
    uint32_t					_width, _height;
    uint8_t						*mData;
  private: 
    xn::ImageMetaData _myMetaData;
};

class ImageSourceKinectDepth : public ImageSource
{
  public:
    ImageSourceKinectDepth( uint16_t *buffer, int width, int height )
      : ImageSource(), mData( buffer ), _width(width), _height(height)
    {
      setSize( _width, _height );
      setColorModel( ImageIo::CM_GRAY );
      setChannelOrder( ImageIo::Y );
      setDataType( ImageIo::UINT16 );
    }

    ~ImageSourceKinectDepth()
    {
    }

    virtual void load( ImageTargetRef target )
    {
      ImageSource::RowFunc func = setupRowFunc( target );

      for( uint32_t row = 0; row < _height; ++row )
        ((*this).*func)( target, row, mData + row * _width );
    }

  protected:
    uint32_t					_width, _height;
    uint16_t					*mData;
};
