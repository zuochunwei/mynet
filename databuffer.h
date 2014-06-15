#pragma once

class data_buffer
{
public:
	data_buffer(unsigned int size);
	data_buffer(unsigned int size, unsigned int size_max);
	data_buffer(const data_buffer&);
	data_buffer& operator= (const data_buffer& rhs);
	~data_buffer();

	unsigned int get_out() const { return _out; }
	void set_out(unsigned int out) { _out = out; }

	unsigned int length() const;
	unsigned int available() const;
	unsigned int available_max() const;

	unsigned int put(const void* from, unsigned int len);
	unsigned int get(void* to, unsigned int len);

	void on_put(unsigned int n) { _in += n; }
	void on_get(unsigned int n) { _out += n; }

	bool full() const { return length() == _size; }
	bool empty() const { return length() == 0; }

	void* get_free_buf(unsigned int& len);
	void* get_data_buf(unsigned int& len);
private:
	bool reserve(unsigned int size);

	unsigned char *_buf;	//buf ptr
	unsigned int _size;		//buf size
	unsigned int _size_max;
	unsigned int _in, _out; //virtual index of _buf
};
