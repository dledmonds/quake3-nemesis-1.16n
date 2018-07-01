#include "q_shared.h"

/*
======================
Q_nextToken - get next delimiter token from in and save into out.
 @in: the string to be searched
 @delimiter: delimiting character
 @out: string to save token into

 return token length
======================
*/
int Q_nextToken( const char *in, const char delimiter, char *out ) {
	int length = 0;

	// copy anything up to delimiter from 'in' into 'out'
	while ( *in && *in != delimiter ) {
		*out++ = *in++;
		length++;
	}
	*out = 0;

	return length;
}

// Imported strpbrk() & strsep() from the GNU C std string library.
char *Q_strpbrk (const char *s, const char *accept)
{
	while (*s != '\0') {
		const char *a = accept;
		while (*a != '\0')
			if (*a++ == *s)
				return (char *) s;
		++s;
	}
	return NULL;
}

char *Q_strsep (char **stringp, const char *delim)
{
  char *begin, *end;

  begin = *stringp;
  if (begin == NULL) {
	  return NULL;
  }

  /* A frequent case is when the delimiter string contains only one
     character.  Here we don't need to call the expensive `strpbrk'
     function and instead work using `strchr'.  */
  if (delim[0] == '\0' || delim[1] == '\0') {
	  char ch = delim[0];
	  
	  if (ch == '\0')
		  end = NULL;
	  else {
		  if (*begin == ch)
			  end = begin;
		  else if (*begin == '\0')
			  end = NULL;
		  else
			  end = strchr (begin + 1, ch);
	  }
  } else
	  /* Find the end of the token.  */
	  end = Q_strpbrk (begin, delim);

  if (end) {
	  /* Terminate the token and set *STRINGP past NUL character.  */
	  *end++ = '\0';
	  *stringp = end;
  } else
	  /* No more delimiters; this is the last token.  */
	  *stringp = NULL;
  
  return begin;
}

qboolean Q_isspace( char c )
{
	if( c == ' ' || c == '\t' || c == '\n' ) {
		return qtrue;
	} else {
		return qfalse;
	}
}

char *Q_strcrop( char *string )
{
    char *last;

    if( string ) {
		last = string + strlen( string );
		while( last > string ) {
			if( !Q_isspace( *( last - 1 ) ) ) {
				break;
			}
			last--;
		}
		*last = 0;
	}
	return( string );
}

int Q_lexncmp(const char *string1, const char *string2, const int count) {
	int cmp;
	char *end;
	
	end = (char *)string1 + count;

	do {
		cmp = (byte) tolower (*string1) - (byte) tolower (*string2);
	} while (*string1++ && *string2++ && cmp == 0 && string1 < end);

	return (cmp);
}

// A case insensitive strstr. Returns a pointer to head of the str1
char *Q_stricstr( const char *str1, const char *str2 ) {
	char *strtmp = (char *)str1;
	int iret = 1;
	
	while (*strtmp) {
		if (strlen (strtmp)>= strlen (str2)) {
			iret = Q_lexncmp (strtmp, str2, strlen (str2));
		} else {
			break;
		}
		
		if (!iret) {
			break;
		}
		
		strtmp++;
	}
	
	return !iret ? strtmp : (char *)NULL;
}
// Case sensitive, all string replacements '#n, etc' must be lowercase due to Q_lexncmp
char *Q_replace( char *strbuf, char *strtofnd, char *strtoins ) {
	char *offset, *strbase;
	
	offset = strbase = (char *)NULL;
	
	while (*strbuf) {
		offset = Q_stricstr (!offset ? strbuf : strbase, strtofnd);
		if (offset) {
			strbase = (offset + strlen (strtoins));
			strcpy (offset, (offset + strlen (strtofnd)));
			memmove (offset + strlen (strtoins), offset, strlen (offset) + 1);
			memcpy (offset, strtoins, strlen (strtoins));
		} else
			break;
	}
	
	return strbuf;
}

