/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./ssl.h"
#include "./fs.h"
#include "./array.h"
#include <limits.h>  // INT_MAX

namespace qk {

	const BIO_METHOD BIOData::method = {
		BIO_TYPE_MEM,
		"quark.js SSL buffer",
		BIOData::Write,
		BIOData::Read,
		BIOData::Puts,
		BIOData::Gets,
		BIOData::Ctrl,
		BIOData::New,
		BIOData::Free,
		nullptr
	};

	BIO* BIOData::New() {
		// The const_cast doesn't violate const correctness.  OpenSSL's usage of
		// BIO_METHOD is effectively const but BIO_new() takes a non-const argument.
		return BIO_new(const_cast<BIO_METHOD*>(&method));
	}

	BIO* BIOData::NewFixed(const char* data, size_t len) {
		BIO* bio = New();

		if (bio == nullptr ||
				len > INT_MAX ||
				BIO_write(bio, data, len) != static_cast<int>(len) ||
				BIO_set_mem_eof_return(bio, 0) != 1) {
			BIO_free(bio);
			return nullptr;
		}

		return bio;
	}

	int BIOData::New(BIO* bio) {
		bio->ptr = new BIOData();

		// XXX Why am I doing it?!
		bio->shutdown = 1;
		bio->init = 1;
		bio->num = -1;

		return 1;
	}


	int BIOData::Free(BIO* bio) {
		if (bio == nullptr)
			return 0;

		if (bio->shutdown) {
			if (bio->init && bio->ptr != nullptr) {
				delete FromBIO(bio);
				bio->ptr = nullptr;
			}
		}

		return 1;
	}


	int BIOData::Read(BIO* bio, char* out, int len) {
		int bytes;
		BIO_clear_retry_flags(bio);

		bytes = FromBIO(bio)->Read(out, len);

		if (bytes == 0) {
			bytes = bio->num;
			if (bytes != 0) {
				BIO_set_retry_read(bio);
			}
		}

		return bytes;
	}


	char* BIOData::Peek(size_t* size) {
		*size = read_head_->write_pos_ - read_head_->read_pos_;
		return read_head_->data_ + read_head_->read_pos_;
	}


	size_t BIOData::PeekMultiple(char** out, size_t* size, size_t* count) {
		Buffer* pos = read_head_;
		size_t max = *count;
		size_t total = 0;

		size_t i;
		for (i = 0; i < max; i++) {
			size[i] = pos->write_pos_ - pos->read_pos_;
			total += size[i];
			out[i] = pos->data_ + pos->read_pos_;

			/* Don't get past write head */
			if (pos == write_head_)
				break;
			else
				pos = pos->next_;
		}

		if (i == max)
			*count = i;
		else
			*count = i + 1;

		return total;
	}


	int BIOData::Write(BIO* bio, const char* data, int len) {
		BIO_clear_retry_flags(bio);

		FromBIO(bio)->Write(data, len);

		return len;
	}


	int BIOData::Puts(BIO* bio, const char* str) {
		return Write(bio, str, strlen(str));
	}


	int BIOData::Gets(BIO* bio, char* out, int size) {
		BIOData* nbio =  FromBIO(bio);

		if (nbio->Length() == 0)
			return 0;

		int i = nbio->IndexOf('\n', size);

		// Include '\n', if it's there.  If not, don't read off the end.
		if (i < size && i >= 0 && static_cast<size_t>(i) < nbio->Length())
			i++;

		// Shift `i` a bit to nullptr-terminate string later
		if (size == i)
			i--;

		// Flush read data
		nbio->Read(out, i);

		out[i] = 0;

		return i;
	}


	long BIOData::Ctrl(BIO* bio, int cmd, long num,  // NOLINT(runtime/int)
										void* ptr) {
		BIOData* nbio;
		long ret;  // NOLINT(runtime/int)

		nbio = FromBIO(bio);
		ret = 1;

		switch (cmd) {
			case BIO_CTRL_RESET:
				nbio->Reset();
				break;
			case BIO_CTRL_EOF:
				ret = nbio->Length() == 0;
				break;
			case BIO_C_SET_BUF_MEM_EOF_RETURN:
				bio->num = num;
				break;
			case BIO_CTRL_INFO:
				ret = nbio->Length();
				if (ptr != nullptr)
					*reinterpret_cast<void**>(ptr) = nullptr;
				break;
			case BIO_C_SET_BUF_MEM:
				Qk_ASSERT(0 && "Can't use SET_BUF_MEM_PTR with BIOData");
				break;
			case BIO_C_GET_BUF_MEM_PTR:
				Qk_ASSERT(0 && "Can't use GET_BUF_MEM_PTR with BIOData");
				ret = 0;
				break;
			case BIO_CTRL_GET_CLOSE:
				ret = bio->shutdown;
				break;
			case BIO_CTRL_SET_CLOSE:
				bio->shutdown = num;
				break;
			case BIO_CTRL_WPENDING:
				ret = 0;
				break;
			case BIO_CTRL_PENDING:
				ret = nbio->Length();
				break;
			case BIO_CTRL_DUP:
			case BIO_CTRL_FLUSH:
				ret = 1;
				break;
			case BIO_CTRL_PUSH:
			case BIO_CTRL_POP:
			default:
				ret = 0;
				break;
		}
		return ret;
	}


	void BIOData::TryMoveReadHead() {
		// `read_pos_` and `write_pos_` means the position of the reader and writer
		// inside the buffer, respectively. When they're equal - its safe to reset
		// them, because both reader and writer will continue doing their stuff
		// from new (zero) positions.
		while (read_head_->read_pos_ != 0 &&
					read_head_->read_pos_ == read_head_->write_pos_) {
			// Reset positions
			read_head_->read_pos_ = 0;
			read_head_->write_pos_ = 0;

			// Move read_head_ forward, just in case if there're still some data to
			// read in the next buffer.
			if (read_head_ != write_head_)
				read_head_ = read_head_->next_;
		}
	}


	size_t BIOData::Read(char* out, size_t size) {
		size_t bytes_read = 0;
		size_t expected = Length() > size ? size : Length();
		size_t offset = 0;
		size_t left = size;

		while (bytes_read < expected) {
			Qk_ASSERT_LE(read_head_->read_pos_, read_head_->write_pos_);
			size_t avail = read_head_->write_pos_ - read_head_->read_pos_;
			if (avail > left)
				avail = left;

			// Copy data
			if (out != nullptr)
				memcpy(out + offset, read_head_->data_ + read_head_->read_pos_, avail);
			read_head_->read_pos_ += avail;

			// Move pointers
			bytes_read += avail;
			offset += avail;
			left -= avail;

			TryMoveReadHead();
		}
		Qk_ASSERT_EQ(expected, bytes_read);
		length_ -= bytes_read;

		// Free all empty buffers, but write_head's child
		FreeEmpty();

		return bytes_read;
	}


	void BIOData::FreeEmpty() {
		if (write_head_ == nullptr)
			return;
		Buffer* child = write_head_->next_;
		if (child == write_head_ || child == read_head_)
			return;
		Buffer* cur = child->next_;
		if (cur == write_head_ || cur == read_head_)
			return;

		Buffer* prev = child;
		while (cur != read_head_) {
			Qk_ASSERT_NE(cur, write_head_);
			Qk_ASSERT_EQ(cur->write_pos_, cur->read_pos_);

			Buffer* next = cur->next_;
			delete cur;
			cur = next;
		}
		prev->next_ = cur;
	}


	size_t BIOData::IndexOf(char delim, size_t limit) {
		size_t bytes_read = 0;
		size_t max = Length() > limit ? limit : Length();
		size_t left = limit;
		Buffer* current = read_head_;

		while (bytes_read < max) {
			Qk_ASSERT_LE(current->read_pos_, current->write_pos_);
			size_t avail = current->write_pos_ - current->read_pos_;
			if (avail > left)
				avail = left;

			// Walk through data
			char* tmp = current->data_ + current->read_pos_;
			size_t off = 0;
			while (off < avail && *tmp != delim) {
				off++;
				tmp++;
			}

			// Move pointers
			bytes_read += off;
			left -= off;

			// Found `delim`
			if (off != avail) {
				return bytes_read;
			}

			// Move to next buffer
			if (current->read_pos_ + avail == current->len_) {
				current = current->next_;
			}
		}
		Qk_ASSERT_EQ(max, bytes_read);

		return max;
	}


	void BIOData::Write(const char* data, size_t size) {
		size_t offset = 0;
		size_t left = size;

		// Allocate initial buffer if the ring is empty
		TryAllocateForWrite(left);

		while (left > 0) {
			size_t to_write = left;
			Qk_ASSERT_LE(write_head_->write_pos_, write_head_->len_);
			size_t avail = write_head_->len_ - write_head_->write_pos_;

			if (to_write > avail)
				to_write = avail;

			// Copy data
			memcpy(write_head_->data_ + write_head_->write_pos_,
						data + offset,
						to_write);

			// Move pointers
			left -= to_write;
			offset += to_write;
			length_ += to_write;
			write_head_->write_pos_ += to_write;
			Qk_ASSERT_LE(write_head_->write_pos_, write_head_->len_);

			// Go to next buffer if there still are some bytes to write
			if (left != 0) {
				Qk_ASSERT_EQ(write_head_->write_pos_, write_head_->len_);
				TryAllocateForWrite(left);
				write_head_ = write_head_->next_;

				// Additionally, since we're moved to the next buffer, read head
				// may be moved as well.
				TryMoveReadHead();
			}
		}
		Qk_ASSERT_EQ(left, 0);
	}


	char* BIOData::PeekWritable(size_t* size) {
		TryAllocateForWrite(*size);

		size_t available = write_head_->len_ - write_head_->write_pos_;
		if (*size != 0 && available > *size)
			available = *size;
		else
			*size = available;

		return write_head_->data_ + write_head_->write_pos_;
	}


	void BIOData::Commit(size_t size) {
		write_head_->write_pos_ += size;
		length_ += size;
		Qk_ASSERT_LE(write_head_->write_pos_, write_head_->len_);

		// Allocate new buffer if write head is full,
		// and there're no other place to go
		TryAllocateForWrite(0);
		if (write_head_->write_pos_ == write_head_->len_) {
			write_head_ = write_head_->next_;

			// Additionally, since we're moved to the next buffer, read head
			// may be moved as well.
			TryMoveReadHead();
		}
	}


	void BIOData::TryAllocateForWrite(size_t hint) {
		Buffer* w = write_head_;
		Buffer* r = read_head_;
		// If write head is full, next buffer is either read head or not empty.
		if (w == nullptr ||
				(w->write_pos_ == w->len_ &&
				(w->next_ == r || w->next_->write_pos_ != 0))) {
			size_t len = w == nullptr ? initial_ :
															kThroughputBufferLength;
			if (len < hint)
				len = hint;
			Buffer* next = new Buffer(len);

			if (w == nullptr) {
				next->next_ = next;
				write_head_ = next;
				read_head_ = next;
			} else {
				next->next_ = w->next_;
				w->next_ = next;
			}
		}
	}


	void BIOData::Reset() {
		if (read_head_ == nullptr)
			return;

		while (read_head_->read_pos_ != read_head_->write_pos_) {
			Qk_ASSERT(read_head_->write_pos_ > read_head_->read_pos_);

			length_ -= read_head_->write_pos_ - read_head_->read_pos_;
			read_head_->write_pos_ = 0;
			read_head_->read_pos_ = 0;

			read_head_ = read_head_->next_;
		}
		write_head_ = read_head_;
		Qk_ASSERT_EQ(length_, 0);
	}


	BIOData::~BIOData() {
		if (read_head_ == nullptr)
			return;

		Buffer* current = read_head_;
		do {
			Buffer* next = current->next_;
			delete current;
			current = next;
		} while (current != read_head_);

		read_head_ = nullptr;
		write_head_ = nullptr;
	}

	// -------------------------------------------------------------------------------
	// NewRootCertStore

	static const char* const root_certs[] = {
		#include "./ssl_certs.h"
	};

	template <typename T, size_t N>
	constexpr size_t arraysize(const T(&)[N]) { return N; }

	static int NoPasswordCallback(char *buf, int size, int rwflag, void *u) {
		return 0;
	}

	static int X509_up_ref(X509* cert) {
		CRYPTO_add(&cert->references, 1, CRYPTO_LOCK_X509);
		return 1;
	}

	X509_STORE* NewRootCertStore() {
		static Array<X509*> root_certs_vector;
		if (root_certs_vector.isNull()) {
			for (size_t i = 0; i < arraysize(root_certs); i++) {
				BIO* bp = BIOData::NewFixed(root_certs[i], strlen(root_certs[i]));
				X509 *x509 = PEM_read_bio_X509(bp, nullptr, NoPasswordCallback, nullptr);
				BIO_free(bp);

				// Parse errors from the built-in roots are fatal.
				Qk_ASSERT_NE(x509, nullptr);

				root_certs_vector.push(x509);
			}
		}

		X509_STORE* store = X509_STORE_new();
#if defined(Qk_OPENSSL_CERT_STORE)
		X509_STORE_set_default_paths(store);
#else
		for (X509 *cert : root_certs_vector) {
			X509_up_ref(cert);
			X509_STORE_add_cert(store, cert);
		}
#endif

		return store;
	}

	X509_STORE* NewRootCertStoreFromFile(cString& ca_content) {
		if (ca_content.isEmpty()) {
			Qk_ELog("%s", "set_ssl_cacert() fail, ca_content is empty string"); return nullptr;
		}

		X509_STORE* store = X509_STORE_new();

		String ssl_cacert_file_path = fs_temp(".cacert.pem");
		fs_write_file_sync(ssl_cacert_file_path, ca_content);

		cChar* ca = fs_fallback_c(ssl_cacert_file_path);

		int r = X509_STORE_load_locations(store, ca, nullptr);
		if (!r) {
			Qk_ELog("%s", "set_ssl_cacert() fail"); return nullptr;
			// Qk_DLog("ssl load x509 store, %s"r);
		}

		return store;
	}

}
