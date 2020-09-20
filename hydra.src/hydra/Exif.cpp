
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Exif.h>

#include <assert.h>

#ifdef BUILD_HYDRA_LIBEXIF_SUPPORT
#include <libexif/exif-data.h>
short hydra::detectExifRotate(const QString &filename) {
    short dm_exif_rotate;

    dm_exif_rotate = 0;
    {
        ExifData *edat;
        ExifEntry *een;
        short num;

        edat = exif_data_new_from_file(filename.toUtf8().constData());
        if (edat && (een = exif_content_get_entry(edat->ifd[EXIF_IFD_0],
                                                  EXIF_TAG_ORIENTATION))) {
            exif_entry_ref(een);

            assert(een->size == 2);
            num = een->data[1] << 8 | een->data[0];
            // num = byte_swap(num);   // byte swap non intel platforms here?
            // OUT << " Orientation! = " << num << '\n';
            if (num == 8)
                dm_exif_rotate = 1;
            if (num == 6)
                dm_exif_rotate = 2;

            exif_entry_unref(een);
            exif_data_free(edat);
        }
    }

    return dm_exif_rotate;
}
#endif

#ifndef BUILD_HYDRA_LIBEXIF_SUPPORT
// mini exif rotate info extractor
#include <QDebug>
#include <QFile>

class CheckedFile : public QFile {
  public:
    class Error : public std::exception {
      public:
        Error(const char *msg) : dm_msg(msg) {}

        virtual const char *what(void) const throw() { return dm_msg; }

      private:
        const char *dm_msg;
    };

  public:
    CheckedFile(const QString &filename) : QFile(filename) {}

    void checkedOpen(OpenMode mode) {
        if (!QFile::open(mode))
            throw Error("Can't open file");
    }

    qint64 checkedRead(unsigned char *data, qint64 maxsize) {
        qint64 r = read(reinterpret_cast<char *>(data), maxsize);
        if (r != maxsize)
            throw Error("checkedRead read");
        return r;
    }

    void checkedAssert(bool exp) {
        if (!exp)
            throw Error("assert failed");
    }

    void checkedSeek(qint64 loc) {
        if (!seek(loc))
            throw Error("seek failed");
    }

    void checkedSeekForward(qint64 forwardbytes) {
        if (!seek(forwardbytes + pos()))
            throw Error("foward-seek failed");
    }
};

inline static unsigned int makeInt(unsigned char *buf, bool little_endian) {
    if (little_endian)
        return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
    else
        return buf[3] + (buf[2] << 8) + (buf[1] << 16) + (buf[0] << 24);
}

inline static unsigned short makeShort(unsigned char *buf, bool little_endian) {
    if (little_endian)
        return buf[0] + (buf[1] << 8);
    else
        return buf[1] + (buf[0] << 8);
}

short hydra::detectExifRotate(const QString &filename) {
    unsigned char buf[32];
    CheckedFile f(filename);
    bool little_endian;

    try {
        f.checkedOpen(QIODevice::ReadOnly);

        // qDebug() << "opening file";

        f.checkedRead(buf, 20);

        // SOI marker
        f.checkedAssert(buf[0] == 0xFF);
        f.checkedAssert(buf[1] == 0xD8);

        // find the APP1 marker
        f.checkedAssert(buf[2] == 0xFF);
        f.checkedAssert(buf[3] == 0XE1);

        // qint64 blocksize = buf[4]*256 + buf[5];

        f.checkedAssert(buf[6] == 'E');
        f.checkedAssert(buf[7] == 'x');
        f.checkedAssert(buf[8] == 'i');
        f.checkedAssert(buf[9] == 'f');
        f.checkedAssert(buf[10] == 0);
        f.checkedAssert(buf[11] == 0);

        // 12 byte order
        // 13 byte order
        if (buf[12] == 0x4D && buf[13] == 0x4D)
            little_endian = false;
        else if (buf[12] == 0x49 && buf[13] == 0x49)
            little_endian = true;
        else
            throw CheckedFile::Error("Can't detect endianess of file");

        // replaced the following with a endian-aware version
        // seemed to be required for the Nikon D80 or something
        // will check spec later to official document this later
        // f.checkedAssert(buf[14] == 0x2A);
        // f.checkedAssert(buf[15] == 0x00);
        short _something_ = makeShort(buf + 14, little_endian);
        f.checkedAssert(_something_ == 0x002A);

        unsigned int ifd0_offset = makeInt(buf + 16, little_endian);

        // qDebug() << "offset = " << ifd0_offset;

        f.checkedSeek(0x0C + ifd0_offset);

        f.checkedRead(buf, 2);

        unsigned short num_fields = makeShort(buf, little_endian);

        // qDebug() << "num_fields " << num_fields;

        for (unsigned short x = 0; x < num_fields; ++x) {
            f.checkedRead(buf, 12);

            if (makeShort(buf, little_endian) == 0x0112) {
                unsigned short orient_code = makeShort(buf + 8, little_endian);

                // 90 left
                if (orient_code == 8)
                    return 3;
                // 90 right
                if (orient_code == 6)
                    return 1;
                return 0;
            }
        }

        return -1;
    } catch (CheckedFile::Error &) {
        // qDebug() << "caugh exception: " << e.what();

        return -1;
    }
}
#endif

/*short hydra::detectExifRotate(const QString &filename)
{
  return 0;
}*/
