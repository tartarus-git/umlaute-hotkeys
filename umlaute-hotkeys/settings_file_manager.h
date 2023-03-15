#pragma once

#define WIN32_LEAN_AND_MEAN		// NOTE: This is ok in this case because we're only using this header for this project.
#include <Windows.h>

#include <io.h>
#include <string>
#include <vector>

#define FILE_CONTENTS_BUFFER_RESIZE_RATE 256

// TODO: make this file do better parsing and handle errors properly.

struct hotkey_t {
	unsigned int fs_modifiers;
	unsigned int virtual_key_code;

	constexpr bool operator==(const hotkey_t& other) const noexcept {
		return fs_modifiers == other.fs_modifiers && virtual_key_code == other.virtual_key_code;
	}
};

struct hotkey_settings_t {
	hotkey_t A;
	hotkey_t O;
	hotkey_t U;
	hotkey_t a;
	hotkey_t o;
	hotkey_t u;
	hotkey_t SS;

	constexpr const hotkey_t& operator[](uint8_t index) const noexcept {
		return *((const hotkey_t*)this + index);
	}

	constexpr hotkey_t& operator[](uint8_t index) noexcept {
		return *((hotkey_t*)this + index);
	}

	constexpr bool operator==(const hotkey_settings_t& other) const noexcept {
		for (uint8_t i = 0; i < sizeof(hotkey_settings_t) / sizeof(hotkey_t); i++) {
			if ((*this)[i] != other[i]) { return false; }
		}
		return true;
	}
};

inline std::string remove_whitespace(const std::string& input) noexcept {
	std::string output;
	for (size_t i = 0; i < input.length(); i++) {
		switch (input[i]) {
		case ' ': continue;
		case '\t': continue;
		}
		output += input[i];
	}
	return output;
}

inline std::vector<std::string> split_string(const std::string& input, char character) noexcept {
	std::vector<std::string> result;
	size_t prev_index = 0;
	for (size_t i = 0; i < input.length(); i++) {
		if (input[i] == character) {
			result.push_back(input.substr(prev_index, i));
			prev_index = i + 1;
		}
	}
	if (prev_index == input.length()) { return result; }
	result.push_back(input.substr(prev_index, input.length()));
	return result;
}

inline char to_lowercase(char character) noexcept {
	if (character >= 'A' && character <= 'Z') {
		return character - ('A' - 'a');
	}
	return character;
}

inline std::string to_lowercase(const std::string& input) noexcept {
	std::string result;
	for (size_t i = 0; i < input.length(); i++) {
		result[i] = to_lowercase(input[i]);
	}
	return result;
}

inline char to_uppercase(char character) noexcept {
	if (character >= 'a' && character <= 'z') {
		return character + ('A' - 'a');
	}
	return character;
}

inline hotkey_t convert_array_to_hotkey(const std::vector<std::string> list) noexcept {
	if (list.size() != 2) { return { 0, 0 }; }

	hotkey_t result;
	result.fs_modifiers = 0;

	// parse modifiers
	std::vector<std::string> modifier_strings = split_string(list[0], '+');
	for (size_t i = 0; i < modifier_strings.size(); i++) {
		std::string lowercase = to_lowercase(modifier_strings[i]);
		if (lowercase == "alt") { result.fs_modifiers |= MOD_ALT; }
		if (lowercase == "control") { result.fs_modifiers |= MOD_CONTROL; }
		if (lowercase == "shift") { result.fs_modifiers |= MOD_SHIFT; }
	}

	// parse virtual key code
	std::string ascii = modifier_strings[1];
	if (ascii.length() == 0) {
		result.virtual_key_code = to_uppercase(ascii[0]);
		return result;
	}
	if (ascii == "SS" || ascii == "ss") {
		result.virtual_key_code = 'S';
		return result;
	}
	return { };
}

inline hotkey_t parse_hotkey_settings_line(const std::string& file_contents, const char *line_tag) noexcept {
	size_t entry_index = file_contents.find(std::string(line_tag) + ": ") + 3;
	size_t endline_index = file_contents.find('\n');
	std::string settings_line = file_contents.substr(entry_index, endline_index);
	std::string no_whitespace_settings_line = remove_whitespace(settings_line);
	std::vector<std::string> split_on_commas = split_string(no_whitespace_settings_line, ',');
	return convert_array_to_hotkey(split_on_commas);
}

inline hotkey_settings_t parse_hotkey_settings(const char* file_path) noexcept {
	int fd = _open(file_path, 0, 0);
	size_t filled_size = 0;
	std::string file_contents;
	while (true) {
		file_contents.resize(filled_size + 256);
		int bytes_read = _read(fd, file_contents.data() + filled_size, 256);
		file_contents.resize(filled_size += bytes_read);
		if (bytes_read == 0) { break; }
		if (bytes_read == -1) { return { }; }
	}
	if (_close(fd) == -1) { return { }; }

	// simple parsing
	hotkey_settings_t result {
		.A = parse_hotkey_settings_line(file_contents, "A"),
		.O = parse_hotkey_settings_line(file_contents, "O"),
		.U = parse_hotkey_settings_line(file_contents, "U"),
		.a = parse_hotkey_settings_line(file_contents, "a"),
		.o = parse_hotkey_settings_line(file_contents, "o"),
		.u = parse_hotkey_settings_line(file_contents, "u"),
		.SS = parse_hotkey_settings_line(file_contents, "SS")
	};

	return result;
}
