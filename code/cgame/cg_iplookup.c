#include "cg_local.h"


#define IPDB_FILESIZE 749376  // size in bytes of ipdb.dat
#define IPDB_FILE "ipdb.dat"


typedef struct {
	unsigned long    range_from_ip;
	unsigned long    range_to_ip;
	unsigned short   num_to_iso2_tld;
} cg_ip_t;


static const char *ipCountryCodes[] = {
   "--", "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AN", "AO",
   "AQ", "AR", "AS", "AT", "AU", "AW", "AZ", "BA", "BB", "BD",
   "BE", "BF", "BG", "BH", "BI", "BJ", "BM", "BN", "BO", "BR",
   "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD", "CF",
   "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR", "CU",
   "CV", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO", "DZ",
   "EC", "EE", "EG", "EH", "ER", "ES", "ET", "FI", "FJ", "FK",
   "FM", "FO", "FR", "FX", "GA", "GD", "GE", "GF", "GG", "GH",
   "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT", "GU",
   "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "ID", "IE",
   "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", "JM",
   "JO", "JP", "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR",
   "KW", "KY", "KZ", "LA", "LB", "LC", "LI", "LK", "LR", "LS",
   "LT", "LU", "LV", "LY", "MA", "MC", "MD", "MG", "MH", "MK",
   "ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU",
   "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE", "NF", "NG",
   "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE",
   "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT",
   "PW", "PY", "QA", "RE", "RO", "RU", "RW", "SA", "SB", "SC",
   "SD", "SE", "SG", "SH", "SI", "SJ", "SK", "SL", "SM", "SN",
   "SO", "SR", "ST", "SV", "SY", "SZ", "TC", "TD", "TF", "TG",
   "TH", "TJ", "TK", "TM", "TN", "TO", "TP", "TR", "TT", "TV",
   "TW", "TZ", "UA", "UG", "UK", "UM", "US", "UY", "UZ", "VA",
   "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT",
   "YU", "ZA", "ZM", "ZW" // total 244
};

static const char *ipCountryNames[] = {
   "Unknown Location", "Andorra", "United Arab Emirates", "Afghanistan", "Antigua and Barbuda",
   "Anguilla", "Albania", "Armenia", "Netherlands Antilles", "Angola",
   "Antarctica", "Argentina", "American Samoa", "Austria", "Australia",
   "Aruba", "Azerbaijan", "Bosnia and Herzegovina", "Barbados", "Bangladesh",
   "Belgium", "Burkina Faso", "Bulgaria", "Bahrain", "Burundi",
   "Benin", "Bermuda", "Brunei Darussalam", "Bolivia", "Brazil",
   "The Bahamas", "Bhutan", "Bouvet Island", "Botswana", "Belarus",
   "Belize", "Canada", "Cocos (Keeling) Islands", "Congo", "Central African Republic",
   "Congo, Republic of the", "Switzerland", "Cote d'Ivoire", "Cook Islands", "Chile",
   "Cameroon", "China", "Colombia", "Costa Rica", "Cuba",
   "Cape Verde", "Christmas Island", "Cyprus", "Czech Republic", "Germany",
   "Djibouti", "Denmark", "Dominica", "Dominican Republic", "Algeria",
   "Ecuador", "Estonia", "Egypt", "Western Sahara", "Eritrea",
   "Spain", "Ethiopia", "Finland", "Fiji", "Falkland Islands (Islas Malvinas)",
   "Micronesia, Federated States of", "Faroe Islands", "France", "France, Metropolitan", "Gabon",
   "Grenada", "Georgia", "French Guiana", "Guernsey", "Ghana",
   "Gibraltar", "Greenland", "The Gambia", "Guinea", "Guadeloupe",
   "Equatorial Guinea", "Greece", "South Georgia and the South Sandwich Islands", "Guatemala", "Guam",
   "Guinea-Bissau", "Guyana", "Hong Kong (SAR)", "Heard Island and McDonald Islands", "Honduras",
   "Croatia", "Haiti", "Hungary", "Indonesia", "Ireland",
   "Israel", "Man, Isle of", "India", "British Indian Ocean Territory", "Iraq",
   "Iran", "Iceland", "Italy", "Jersey", "Jamaica",
   "Jordan", "Japan", "Kenya", "Kyrgyzstan", "Cambodia",
   "Kiribati", "Comoros", "Saint Kitts and Nevis", "Korea, North", "Korea, South",
   "Kuwait", "Cayman Islands", "Kazakhstan", "Laos", "Lebanon",
   "Saint Lucia", "Liechtenstein", "Sri Lanka", "Liberia", "Lesotho",
   "Lithuania", "Luxembourg", "Latvia", "Libya", "Morocco",
   "Monaco", "Moldova", "Madagascar", "Marshall Islands", "Macedonia",
   "Mali", "Burma", "Mongolia", "Macao", "Northern Mariana Islands",
   "Martinique", "Mauritania", "Montserrat", "Malta", "Mauritius",
   "Maldives", "Malawi", "Mexico", "Malaysia", "Mozambique",
   "Namibia", "New Caledonia", "Niger", "Norfolk Island", "Nigeria",
   "Nicaragua", "Netherlands", "Norway", "Nepal", "Nauru",
   "Niue", "New Zealand", "Oman", "Panama", "Peru",
   "French Polynesia", "Papua New Guinea", "Philippines", "Pakistan", "Poland",
   "Saint Pierre and Miquelon", "Pitcairn Islands", "Puerto Rico", "Palestinian Territory, Occupied", "Portugal",
   "Palau", "Paraguay", "Qatar", "Réunion", "Romania",
   "Russia", "Rwanda", "Saudi Arabia", "Solomon Islands", "Seychelles",
   "Sudan", "Sweden", "Singapore", "Saint Helena", "Slovenia",
   "Svalbard", "Slovakia", "Sierra Leone", "San Marino", "Senegal",
   "Somalia", "Suriname", "São Tomé and Príncipe", "El Salvador", "Syria",
   "Swaziland", "Turks and Caicos Islands", "Chad", "French Southern and Antarctic Lands", "Togo",
   "Thailand", "Tajikistan", "Tokelau", "Turkmenistan", "Tunisia",
   "Tonga", "East Timor", "Turkey", "Trinidad and Tobago", "Tuvalu",
   "Taiwan", "Tanzania", "Ukraine", "Uganda", "United Kingdom",
   "United States Minor Outlying Islands", "United States", "Uruguay", "Uzbekistan", "Holy See (Vatican City)",
   "Saint Vincent and the Grenadines", "Venezuela", "British Virgin Islands", "Virgin Islands", "Vietnam",
   "Vanuatu", "Wallis and Futuna", "Samoa", "Yemen", "Mayotte",
   "Yugoslavia", "South Africa", "Zambia", "Zimbabwe"
};

// not allowed locals over 32k, so this has to be global
static cg_ip_t ips[IPDB_FILESIZE / sizeof(cg_ip_t)];


/*
=============
CG_LookupCountryCode

Gets the 2 digit country code for a given TLD
=============
*/
const char* CG_LookupCountryCode(int tld) {
	if (tld > 0 || tld < sizeof(ipCountryCodes)/sizeof(ipCountryCodes[0])) {
		return ipCountryCodes[tld];
	}

	return ipCountryCodes[0];
}


/*
=============
CG_LookupCountryName

Gets the 2 digit country name for a given TLD
=============
*/
const char* CG_LookupCountryName(int tld) {
	if (tld > 0 || tld < sizeof(ipCountryNames)/sizeof(ipCountryNames[0])) {
		return ipCountryNames[tld];
	}

	return ipCountryNames[0];
}


/*
=============
CG_ConvertIP

Converts an IP string into an unsigned long representation

Returns converted IP or 0 if invalid
=============
*/
static unsigned long CG_ConvertIP(char* ipStr) {
	char *tmp;
	int dotCount, i;
	unsigned long ipNum;
	short ipArr[4][3];
	int total;

	dotCount = 0;
	ipNum = 0;
	tmp = ipStr;
	total = 0;
	memset(ipArr, -1, sizeof(ipArr));

	while (*tmp) {
		if (*tmp == '.') {	
			if (++dotCount > 3 || ipArr[dotCount-1][0] == -1) {
				CG_Printf("Invalid IP - must be of the format xxx.xxx.xxx.xxx\n");
				return 0;
			}

			tmp++;
			continue;
		}

		// only allowed 0-9
		if (*tmp < 48 || *tmp > 57) {
			CG_Printf("Invalid IP - can only contain digits 0-9 and .\n");
			return 0;
		}

		// gather digits
		if (ipArr[dotCount][0] == -1) {
			ipArr[dotCount][0] = *tmp - 48;
		} else if (ipArr[dotCount][1] == -1) {
			ipArr[dotCount][1] = *tmp - 48;
		} else if (ipArr[dotCount][2] == -1) {
			ipArr[dotCount][2] = *tmp - 48;
		} else {
			// too many digits
			CG_Printf("Invalid IP - maximum of 3 digits per quad\n");
			return 0;
		}

		tmp++;
	}

	if (dotCount != 3) {
		CG_Printf("Invalid IP - must be of the format xxx.xxx.xxx.xxx\n");
		return 0;
	}

	// work out the long representation
	for (i = 0; i < 4; i++) {
		if (ipArr[i][2] == -1) {
			if (ipArr[i][1] == -1) {
				total = ipArr[i][0];
			} else {
				total = (ipArr[i][0] * 10) + ipArr[i][1];
			}
		} else {
			total = (ipArr[i][0] * 100) + (ipArr[i][1] * 10) + ipArr[i][2];
		}

		if (total > 255) {
			CG_Printf("Invalid IP - quad %i must between in the range 0 - 255\n", total);
			return 0;
		}

		ipNum += total << ((3-i)*8);  // shift into the right location
	}

	return ipNum;
}


/*
=============
CG_LookupIP

Performs a county lookup from IP address, given an IP as a string

Returns the TLD code or -1 on error
=============
*/
int CG_LookupIP(char* ipStr) {
	fileHandle_t	f;
	cg_ip_t			*ip;
	int				length, count;
	int				i;
	unsigned long	ipNum;

// can only do full debug this in dll as QVMs moan about the data size
/*#ifdef WIN32
	unsigned short	to1, to2, to3, to4;
	unsigned short	from1, from2, from3, from4;
#endif*/

	ipNum = CG_ConvertIP(ipStr);
	if (!ipNum) {
		return -1;
	}

	// try to open the file
	if ((length = trap_FS_FOpenFile(IPDB_FILE, &f, FS_READ)) <= 0) {
        return -1;
    }
	if (length <= 0) {
		trap_FS_FCloseFile(f);
		return -1;
	}

	trap_FS_Read(ips, length, f);
	trap_FS_FCloseFile(f);

	count = length / sizeof(ips[0]);
	for (i=0; i<count; i++) {
		ip = &ips[i];

// can only do full debug this in dll as QVMs moan about the data size
/*#ifdef WIN32
		from1 = (ip->range_from_ip & 0xff000000) >> 24;
		from2 = (ip->range_from_ip & 0x00ff0000) >> 16;
		from3 = (ip->range_from_ip & 0x0000ff00) >> 8;
		from4 = (ip->range_from_ip & 0x000000ff);

		to1 = (ip->range_to_ip & 0xff000000) >> 24;
		to2 = (ip->range_to_ip & 0x00ff0000) >> 16;
		to3 = (ip->range_to_ip & 0x0000ff00) >> 8;
		to4 = (ip->range_to_ip & 0x000000ff);
#endif */
		if (ip->range_from_ip < ipNum && ipNum < ip->range_to_ip) {
/*#ifdef WIN32
			CG_Printf("Match in ^3%i.%i.%i.%i ^7- ^3%i.%i.%i.%i (%d)\n", 
				from1, from2, from3, from4, 
				to1, to2, to3, to4, 
				ip->num_to_iso2_tld);
#else
			CG_Printf("Match found ^3%d\n", ip->num_to_iso2_tld);
#endif*/

			return ip->num_to_iso2_tld;
		}
	}

	return -1;
}
