// VTSPacketDB.cpp: implementation of the VTSPacketDB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vts.h"
#include "VTSPacketDB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


const int kVTSDBMajorVersion = 3;			// current version
const int kVTSDBMinorVersion = 3;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VTSPacketDB::VTSPacketDB(void)
{
	m_pfilePackets = NULL;
}


VTSPacketDB::~VTSPacketDB( void )
{
	if ( m_pfilePackets != NULL )
		delete m_pfilePackets;
}


bool VTSPacketDB::Open( LPCTSTR lpszFileName )
{
	CFile * pfileNew = new CFile();

	if ( pfileNew == NULL )
		return false;

	CFileException e;
	CString strError;
	DWORD	dwCookie = 0xBAC058AD;		// cookie for file ID

	if ( !pfileNew->Open( lpszFileName, CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareExclusive, &e ) )
	{
		TCHAR   szCause[255];
		// File could not be opened... see if we can create it first...

		if ( !pfileNew->Open( lpszFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareExclusive, &e ) )
		{
			e.GetErrorMessage(szCause, 255);
			strError.Format(IDS_ERR_FILEPKTOPEN, lpszFileName);
		    strError += szCause;
		}
		else
		{
			// create cookie and set file position...
			pfileNew->Write(&dwCookie, sizeof(DWORD));
			pfileNew->SetLength(sizeof(DWORD));
			pfileNew->Flush();
		}
	}
	else
	{
		// file was opened successfully... now check for cookie
		// file position is left after cookie.

		DWORD dwCookieTest;
		if ( pfileNew->Read(&dwCookieTest, sizeof(DWORD)) != sizeof(DWORD)  ||  dwCookieTest != dwCookie )
		{
			strError.Format(IDS_ERR_FILENOTPKT, lpszFileName);
			pfileNew->Close();
		}
	}

	if ( !strError.IsEmpty() )
	{
		AfxMessageBox(strError);
		return false;
	}

	// We're messing with file pointers so lock out other threads before this write
	CSingleLock lock( &writeLock );
	lock.Lock();

	Close();

	m_strPacketFileName = lpszFileName;
	m_pfilePackets = pfileNew;
	lock.Unlock();

	return true;
}



void VTSPacketDB::Close()
{
	if ( m_pfilePackets != NULL )
	{
		if ( m_pfilePackets->m_hFile != CFile::hFileNull )  // MAG 01FEB06 removed (UINT) cast from hFileNull
		{
			try
			{
				m_pfilePackets->Flush();
				m_pfilePackets->Close();
			}
			catch (CFileException e )
			{}
		}

		delete m_pfilePackets;
		m_pfilePackets = NULL;
	}

	m_strPacketFileName.Empty();
}



void VTSPacketDB::DeletePackets( void )
{
	// We're messing with file pointers so lock out other threads before this write
	CSingleLock lock( &writeLock );
	lock.Lock();

	try
	{
		m_pfilePackets->SetLength(sizeof(DWORD));		// beginning of file is after cookie
		m_pfilePackets->Flush();
		m_pfilePackets->SeekToEnd();
	}
	catch (CFileException e)
	{
	}

	lock.Unlock();
}



// returns -1 if file problem or no more records

LONG VTSPacketDB::ReadNextPacket(VTSPacket& pkt, LONG lPosition /* = -1 */  )
{
	ASSERT(m_pfilePackets != NULL && m_pfilePackets->m_hFile != CFile::hFileNull );  // MAG 01FEB06 removed (UINT) cast from hFileNull

	if ( m_pfilePackets == NULL || m_pfilePackets->m_hFile == CFile::hFileNull )  // MAG 01FEB06 removed (UINT) cast from hFileNull
		return -1;

	// make sure no other threads are mucking around
	CSingleLock lock( &writeLock );
	lock.Lock();

	// Get current position to restore if read problem
	LONG nCurrentPosition = m_pfilePackets->Seek(0, CFile::current);
	BACnetOctetPtr pbuffer = NULL;
	LONG nNewPosition = -1;

	// If caller specified a position to read from, go there first... 
	// otherwise, use the current position

	try
	{
		if ( lPosition != -1 )
			m_pfilePackets->Seek( lPosition < sizeof(DWORD) ? sizeof(DWORD) : lPosition, CFile::begin);

		// Attempt to read the packet header... throw exception if header isn't there
		if ( m_pfilePackets->Read(&pkt.packetHdr, sizeof(pkt.packetHdr)) != sizeof(pkt.packetHdr) )
			AfxThrowFileException(CFileException::endOfFile);

		// Attempt to read the size of the octets... throw exception if size isn't there
		if ( m_pfilePackets->Read(&pkt.packetLen, sizeof(int)) != sizeof(int) )
			AfxThrowFileException(CFileException::endOfFile);

		// OK...  See if there is any octet data.  If not, buffer will already be NULL
		// and length will be zero.

		if ( pkt.packetLen != 0 )
		{
			// Yep.  Allocate size for data and pull it from the file
			pbuffer = new BACnetOctet[pkt.packetLen];		// ought to be enough

			if ( pbuffer == NULL )
				AfxThrowFileException(CFileException::none);

			if ( m_pfilePackets->Read(pbuffer, pkt.packetLen) != (UINT) pkt.packetLen )
				AfxThrowFileException(CFileException::endOfFile);
		}

		// pull off remaining number of bytes read
		// Doesn't matter if blows here

		int nBogus;
		m_pfilePackets->Read(&nBogus, sizeof(int));

		nNewPosition = m_pfilePackets->Seek(0, CFile::current);

		// Point to new octet buffer and make the packet own the stuff so it kills it later
		// Don't assign it until all file operations have completed (so we don't have to undo the assignment).

		pkt.NewDataRef(pbuffer, pkt.packetLen, true);
	}
	catch(...)
	{
		// A problem ocurred while reading the file...  Try to clean up
		if ( pbuffer != NULL )
		{
			pkt.packetLen = 0;
			delete pbuffer;
		}

		// reset file pointer and return read error or end of file (-1)
		m_pfilePackets->Seek(nCurrentPosition, CFile::begin);
	}

	// be nice and release it before returning
	lock.Unlock();
	return nNewPosition;
}



LONG VTSPacketDB::WritePacket( VTSPacket& pkt )
{
	if ( m_pfilePackets == NULL )
		return 0;

	// We're messing with file pointers so lock out other threads before this write
	CSingleLock lock( &writeLock );
	lock.Lock();

	DWORD	dwFileLength = m_pfilePackets->SeekToEnd();

	try
	{
		m_pfilePackets->Write(&pkt.packetHdr, sizeof(pkt.packetHdr));
		m_pfilePackets->Write(&pkt.packetLen, sizeof(int));

		if ( pkt.packetLen != 0 )
			m_pfilePackets->Write(pkt.packetData, pkt.packetLen);

		// now write the total length of all this data so we can pull it out later
		// This allows us to move backwards through the file one record at a time

		int nTotal = sizeof(pkt.packetHdr) + sizeof(int) + pkt.packetLen;
		m_pfilePackets->Write(&nTotal, sizeof(int));
	}
	catch (CFileException e)
	{
		// There was a problem writing one of these guys... rollback the file and return the error
		m_pfilePackets->SetLength(dwFileLength);
		m_pfilePackets->Flush();
		// return an error
	}

	lock.Unlock();
	return dwFileLength;		// return file position where this record was written
}

