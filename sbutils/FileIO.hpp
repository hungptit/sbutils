#pragma once

// This header has fast functions for reading a file in Linux.

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <string>
#include <iostream>

namespace sbutils {
	template<typename Container>
	Container read_all(const std::string &fname) {
		Container data;
		int answer = 0;
		int fd = open(fname.c_str(), O_RDONLY);
		if (fd < 0) {
			throw(std::runtime_error("Cannot read" + fname));
		}
		
		// TODO: Check at compile time that Container::value_type is one byte.
		size_t file_size = ULLONG_MAX;
		off_t errcode = lseek(fd, file_size, SEEK_END);
		
		
		close(fd);
	}
}
