// VendorIdTable.cpp : Extract VendorID strings from the web page
// at http://www.bacnet.org/VendorID/BACnet%20Vendor%20IDs.htm
// (or at least, from a file gotten from there)
//
// Written by John Hartman
//

#include "stdafx.h"

void OutputLine( FILE *pOutput, const char *pVendorName, int vendorID )
{
	// Replace &amp; <BR> &#146; &#228; etc.
	// This is NOT a complete swapper
	// (<BR occurs once, presumably by accident...)
	char buf[ 100 ];
	char *pOut = buf;
	const char *pName = pVendorName;
	char *pNext;
	unsigned int ampChar;
	while (*pName)
	{
		if (*pName == '&')
		{
			if (memcmp(pName, "&amp;", 5) == 0)
			{
				pName += 5;
			}
			else if (pName[1] == '#')
			{
				ampChar = strtoul( pName+2, &pNext, 10 );
				if (*pNext == ';')
				{
					pName = pNext + 1;
					*pOut++ = (char)ampChar;
				}
				else
				{
					fprintf( stderr, "Unknown escape sequence in %s\n", pVendorName );
					exit(-1);
				}
			}
			else
			{
				fprintf( stderr, "Unknown escape sequence in %s\n", pVendorName );
				exit(-1);
			}
		}
		else if (memcmp(pName, "<BR>", 4) == 0)
		{
			pName += 4;
		}
		else
		{
			*pOut++ = *pName++;
		}
	}
	*pOut = 0;

	int column = fprintf( pOutput, "    \"%s\",", buf );
	while (column < 60)
	{
		column += fprintf( pOutput, " " );
	}

	fprintf( pOutput, " // %d\n", vendorID );
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		printf( "VendorIdTable: Generate a table of VendorID strings from a file\n"
			    "usage:\n"
				"  Use a browser to get and save the table from\n"
                "     http://www.bacnet.org/VendorID/BACnet%20Vendor%20IDs.htm\n"
				"  VendorIdTable file.htm file.txt\n"
				"Output in file.txt will be a list of vendorID string suitable for use\n"
				"as the table BACnetVendorID in vts StringTables.cpp\n" );
		return 1;
	}

	FILE *pInput = fopen( argv[1], "r" );
	if (pInput == NULL)
	{
		fprintf( stderr, "Cannot open input file %s\n", argv[1] );
		return 2;
	}
	
	FILE *pOutput = fopen( argv[2], "w" );
	if (pOutput == NULL)
	{
		fprintf( stderr, "Cannot open output file %s\n", argv[2] );
		return 3;
	}

	// The HTML file consists mainly of a table.
	// We extract the first two <TD> from each row, using a really crude method...
	// <TR>
	// <TH>Vendor ID</TH>
	// <TH>Organization</TH>
	// <TH>Contact Person</TH>
	// <TH>Address</TH>
	// </TR>
	// <TR>
	// <TD>0</TD>
	// <TD>ASHRAE</TD>
	// <TD>Stephanie Reiniche, MOS</TD>
	// <TD>1791 Tullie Circle, N.E.<BR>Atlanta, GA 30329</TD>
	// </TR>
	// <TR>
	// <TD>1</TD>
	// <TD>NIST</TD>
	// <TD>Steven T. Bushby</TD>
	// <TD>Mechanical Systems and Controls<BR>Building 226, Room B114<BR>Gaithersburg, MD 20899</TD>
	// </TR>
	int state = 0;
	char buffer[ 1000 ];
	int  vendorID;
	int  nextID = 0;
	while (fgets( buffer, sizeof(buffer), pInput ) != NULL)
	{
		if (strnicmp( buffer, "<TR>", 4 ) == 0)
		{
			// Start of a row
			state = 1;
		}
		else if (strnicmp( buffer, "</TR>", 5 ) == 0)
		{
			// End of a row
			state = 0;
		}
		else if (strnicmp( buffer, "<TD>", 4 ) == 0)
		{
			// Start of a data item
			char *pEnd = strstr( &buffer[4], "</TD>" );
			if (pEnd == NULL)
			{
				// Probably a problem
				fprintf( stderr, "Missing expected </TD> in %s\n", buffer );
				state = 0;
			}
			else
			{
				*pEnd = 0;
				if (state == 1)
				{
					// Vendor ID
					if (sscanf( &buffer[4], "%d", &vendorID ) != 1)
					{
						fprintf( stderr, "Missing expected vendorID in %s\n", buffer );
						state = 0;
					}
					else
					{
						state = 2;
					}

					// The table is missing at least one code (at 177),
					// plus one or more reserved by SSPC135
					for ( ; nextID < vendorID; nextID++)
					{
						if (nextID == 555)
						{
							// SSPC135 has reserved this for use in examples, following
							// the example of move phone numbers as 555-1234
							OutputLine( pOutput, "BACnet Examples", nextID );
						}
						else if (nextID == 666)
						{
							// SSPC135 has reserved this for use in examples
							// of badly-behaved devices
							OutputLine( pOutput, "Beelzebub Controls", nextID );
						}
						else
						{
							char buf[ 80 ];
							sprintf (buf, "Unknown-vendor-%d", nextID );
							OutputLine( pOutput, buf, nextID );
						}
					}

					if (vendorID != nextID)
					{
						fprintf( stderr, "VendorID %d differs from expected ID %d\n", vendorID, nextID );
					}

					nextID += 1;
				}
				
				else if (state == 2)
				{
					// Vendor Name
					OutputLine( pOutput, &buffer[4], vendorID );
					state = 0;
				}
			}
		}
		else if (strstr( buffer, "</TD>" ) != NULL)
		{
			// End of a data item
			state += 1;
		}
	}

	fclose( pInput );
	fclose( pOutput );
	
	return 0;
}

