

inline bool	operator ==(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart == b.QuadPart);
}

inline bool operator != (const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart != b.QuadPart);
}

inline bool	operator <=(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart <= b.QuadPart);
}

inline bool	operator >=(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart >= b.QuadPart);
}

inline bool	operator <(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart < b.QuadPart);
}

inline bool	operator >(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	return (a.QuadPart > b.QuadPart);
}

inline bool	operator <(const DWORD &a, const ULARGE_INTEGER &b)
{
	return (a < b.QuadPart);
}

inline bool	operator >(const DWORD &a, const ULARGE_INTEGER &b)
{
	return (a > b.QuadPart);
}

inline bool	operator <=(const DWORD &a, const ULARGE_INTEGER &b)
{
	return (a <= b.QuadPart);
}

inline bool	operator >=(const DWORD &a, const ULARGE_INTEGER &b)
{
	return (a >= b.QuadPart);
}

inline bool	operator <(const ULARGE_INTEGER &a, const DWORD &b)
{
	return (a.QuadPart < b);
}

inline bool	operator >(const ULARGE_INTEGER &a, const DWORD &b)
{
	return (a.QuadPart > b);
}

inline bool	operator <=(const ULARGE_INTEGER &a, const DWORD &b)
{
	return (a.QuadPart <= b);
}

inline bool	operator >=(const ULARGE_INTEGER &a, const DWORD &b)
{
	return (a.QuadPart >= b);
}

inline ULARGE_INTEGER operator +(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	ULARGE_INTEGER c;
	c.QuadPart = a.QuadPart + b.QuadPart;
	return c;
}

inline ULARGE_INTEGER operator -(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	ULARGE_INTEGER c;
	c.QuadPart = a.QuadPart - b.QuadPart;
	return c;
}

inline ULARGE_INTEGER operator /(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	ULARGE_INTEGER c;
	c.QuadPart = a.QuadPart / b.QuadPart;
	return c;
}

inline ULARGE_INTEGER operator *(const ULARGE_INTEGER &a, const ULARGE_INTEGER &b)
{
	ULARGE_INTEGER c;
	c.QuadPart = a.QuadPart * b.QuadPart;
	return c;
}


