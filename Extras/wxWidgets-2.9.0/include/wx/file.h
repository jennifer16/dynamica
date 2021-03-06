/////////////////////////////////////////////////////////////////////////////
// Name:        file.h
// Purpose:     wxFile - encapsulates low-level "file descriptor"
//              wxTempFile - safely replace the old file
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29/01/98
// RCS-ID:      $Id: file.h 55909 2008-09-27 10:28:43Z FM $
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEH__
#define _WX_FILEH__

#include  "wx/defs.h"

#if wxUSE_FILE

#include  "wx/string.h"
#include  "wx/filefn.h"
#include  "wx/strconv.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// we redefine these constants here because S_IREAD &c are _not_ standard
// however, we do assume that the values correspond to the Unix umask bits
enum wxPosixPermissions
{
    // standard Posix names for these permission flags:
    wxS_IRUSR = 00400,
    wxS_IWUSR = 00200,
    wxS_IXUSR = 00100,

    wxS_IRGRP = 00040,
    wxS_IWGRP = 00020,
    wxS_IXGRP = 00010,

    wxS_IROTH = 00004,
    wxS_IWOTH = 00002,
    wxS_IXOTH = 00001,

    // longer but more readable synonims for the constants above:
    wxPOSIX_USER_READ = wxS_IRUSR,
    wxPOSIX_USER_WRITE = wxS_IWUSR,
    wxPOSIX_USER_EXECUTE = wxS_IXUSR,

    wxPOSIX_GROUP_READ = wxS_IRGRP,
    wxPOSIX_GROUP_WRITE = wxS_IWGRP,
    wxPOSIX_GROUP_EXECUTE = wxS_IXGRP,

    wxPOSIX_OTHERS_READ = wxS_IROTH,
    wxPOSIX_OTHERS_WRITE = wxS_IWOTH,
    wxPOSIX_OTHERS_EXECUTE = wxS_IXOTH,

    // default mode for the new files: allow reading/writing them to everybody but
    // the effective file mode will be set after anding this value with umask and
    // so won't include wxS_IW{GRP,OTH} for the default 022 umask value
    wxS_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | \
                   wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | \
                   wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE),

    // default mode for the new directories (see wxFileName::Mkdir): allow
    // reading/writing/executing them to everybody, but just like wxS_DEFAULT
    // the effective directory mode will be set after anding this value with umask
    wxS_DIR_DEFAULT = (wxPOSIX_USER_READ | wxPOSIX_USER_WRITE | wxPOSIX_USER_EXECUTE | \
                       wxPOSIX_GROUP_READ | wxPOSIX_GROUP_WRITE | wxPOSIX_GROUP_EXECUTE | \
                       wxPOSIX_OTHERS_READ | wxPOSIX_OTHERS_WRITE | wxPOSIX_OTHERS_EXECUTE)
};

// ----------------------------------------------------------------------------
// class wxFile: raw file IO
//
// NB: for space efficiency this class has no virtual functions, including
//     dtor which is _not_ virtual, so it shouldn't be used as a base class.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFile
{
public:
  // more file constants
  // -------------------
    // opening mode
  enum OpenMode { read, write, read_write, write_append, write_excl };
    // standard values for file descriptor
  enum { fd_invalid = -1, fd_stdin, fd_stdout, fd_stderr };

  // static functions
  // ----------------
    // check whether a regular file by this name exists
  static bool Exists(const wxString& name);
    // check whether we can access the given file in given mode
    // (only read and write make sense here)
  static bool Access(const wxString& name, OpenMode mode);

  // ctors
  // -----
    // def ctor
  wxFile() { m_fd = fd_invalid; m_error = false; }
    // open specified file (may fail, use IsOpened())
  wxFile(const wxString& fileName, OpenMode mode = read);
    // attach to (already opened) file
  wxFile(int lfd) { m_fd = lfd; m_error = false; }

  // open/close
    // create a new file (with the default value of bOverwrite, it will fail if
    // the file already exists, otherwise it will overwrite it and succeed)
  bool Create(const wxString& fileName, bool bOverwrite = false,
              int access = wxS_DEFAULT);
  bool Open(const wxString& fileName, OpenMode mode = read,
            int access = wxS_DEFAULT);
  bool Close();  // Close is a NOP if not opened

  // assign an existing file descriptor and get it back from wxFile object
  void Attach(int lfd) { Close(); m_fd = lfd; m_error = false; }
  void Detach()       { m_fd = fd_invalid;  }
  int  fd() const { return m_fd; }

  // read/write (unbuffered)
    // returns number of bytes read or wxInvalidOffset on error
  ssize_t Read(void *pBuf, size_t nCount);
    // returns the number of bytes written
  size_t Write(const void *pBuf, size_t nCount);
    // returns true on success
  bool Write(const wxString& s, const wxMBConv& conv = wxMBConvUTF8());
    // flush data not yet written
  bool Flush();

  // file pointer operations (return wxInvalidOffset on failure)
    // move ptr ofs bytes related to start/current offset/end of file
  wxFileOffset Seek(wxFileOffset ofs, wxSeekMode mode = wxFromStart);
    // move ptr to ofs bytes before the end
  wxFileOffset SeekEnd(wxFileOffset ofs = 0) { return Seek(ofs, wxFromEnd); }
    // get current offset
  wxFileOffset Tell() const;
    // get current file length
  wxFileOffset Length() const;

  // simple accessors
    // is file opened?
  bool IsOpened() const { return m_fd != fd_invalid; }
    // is end of file reached?
  bool Eof() const;
    // has an error occurred?
  bool Error() const { return m_error; }
    // type such as disk or pipe
  wxFileKind GetKind() const { return wxGetFileKind(m_fd); }

  // dtor closes the file if opened
  ~wxFile() { Close(); }

private:
  // copy ctor and assignment operator are private because
  // it doesn't make sense to copy files this way:
  // attempt to do it will provoke a compile-time error.
  wxFile(const wxFile&);
  wxFile& operator=(const wxFile&);

  int m_fd; // file descriptor or INVALID_FD if not opened
  bool m_error; // error memory
};

// ----------------------------------------------------------------------------
// class wxTempFile: if you want to replace another file, create an instance
// of wxTempFile passing the name of the file to be replaced to the ctor. Then
// you can write to wxTempFile and call Commit() function to replace the old
// file (and close this one) or call Discard() to cancel the modification. If
// you call neither of them, dtor will call Discard().
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxTempFile
{
public:
  // ctors
    // default
  wxTempFile() { }
    // associates the temp file with the file to be replaced and opens it
  wxTempFile(const wxString& strName);

  // open the temp file (strName is the name of file to be replaced)
  bool Open(const wxString& strName);

  // is the file opened?
  bool IsOpened() const { return m_file.IsOpened(); }
    // get current file length
  wxFileOffset Length() const { return m_file.Length(); }
    // move ptr ofs bytes related to start/current offset/end of file
  wxFileOffset Seek(wxFileOffset ofs, wxSeekMode mode = wxFromStart)
    { return m_file.Seek(ofs, mode); }
    // get current offset
  wxFileOffset Tell() const { return m_file.Tell(); }

  // I/O (both functions return true on success, false on failure)
  bool Write(const void *p, size_t n) { return m_file.Write(p, n) == n; }
  bool Write(const wxString& str, const wxMBConv& conv = wxMBConvUTF8())
    { return m_file.Write(str, conv); }

  // different ways to close the file
    // validate changes and delete the old file of name m_strName
  bool Commit();
    // discard changes
  void Discard();

  // dtor calls Discard() if file is still opened
 ~wxTempFile();

private:
  // no copy ctor/assignment operator
  wxTempFile(const wxTempFile&);
  wxTempFile& operator=(const wxTempFile&);

  wxString  m_strName,  // name of the file to replace in Commit()
            m_strTemp;  // temporary file name
  wxFile    m_file;     // the temporary file
};

#endif // wxUSE_FILE

#endif // _WX_FILEH__
