

inline bool	operator ==(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart == b.QuadPart);
}

inline bool operator != (const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart != b.QuadPart);
}

inline bool	operator <=(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart <= b.QuadPart);
}

inline bool	operator >=(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart >= b.QuadPart);
}

inline bool	operator <(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart < b.QuadPart);
}

inline bool	operator >(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	return (a.QuadPart > b.QuadPart);
}

inline bool	operator <(const long &a, const LARGE_INTEGER &b)
{
	return (a < b.QuadPart);
}

inline bool	operator >(const long &a, const LARGE_INTEGER &b)
{
	return (a > b.QuadPart);
}

inline bool	operator <=(const long &a, const LARGE_INTEGER &b)
{
	return (a <= b.QuadPart);
}

inline bool	operator >=(const long &a, const LARGE_INTEGER &b)
{
	return (a >= b.QuadPart);
}

inline bool	operator <(const LARGE_INTEGER &a, const long &b)
{
	return (a.QuadPart < b);
}

inline bool	operator >(const LARGE_INTEGER &a, const long &b)
{
	return (a.QuadPart > b);
}

inline bool	operator <=(const LARGE_INTEGER &a, const long &b)
{
	return (a.QuadPart <= b);
}

inline bool	operator >=(const LARGE_INTEGER &a, const long &b)
{
	return (a.QuadPart >= b);
}

inline LARGE_INTEGER operator +(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	LARGE_INTEGER c;
	c.QuadPart = a.QuadPart + b.QuadPart;
	return c;
}

inline LARGE_INTEGER operator -(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	LARGE_INTEGER c;
	c.QuadPart = a.QuadPart - b.QuadPart;
	return c;
}

inline LARGE_INTEGER operator /(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	LARGE_INTEGER c;
	c.QuadPart = a.QuadPart / b.QuadPart;
	return c;
}

inline LARGE_INTEGER operator *(const LARGE_INTEGER &a, const LARGE_INTEGER &b)
{
	LARGE_INTEGER c;
	c.QuadPart = a.QuadPart * b.QuadPart;
	return c;
}
