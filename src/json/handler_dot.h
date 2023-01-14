//
//
//

#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "handler.h"


namespace json {
namespace handler {

	class handler_dot_t : public handler_t {
		class context_t {
			bool        m_arrayFlag;
			int         m_arrayIndex;
			std::string m_path;
			std::string m_name;

		public:
			context_t(bool isArray, const std::string& path, const std::string& name) :
				m_arrayFlag(isArray),
				m_arrayIndex(0),
				m_path(path),
				m_name(name) {}
			// copy constructor
			context_t(const context_t& that) :
				m_arrayFlag(that.m_arrayFlag),
				m_arrayIndex(that.m_arrayIndex),
				m_path(that.m_path),
				m_name(that.m_name) {}
			// move constructor
			context_t(context_t&& that) noexcept :
				m_arrayFlag(that.m_arrayFlag),
				m_arrayIndex(that.m_arrayIndex),
				m_path(std::move(that.m_path)),
				m_name(std::move(that.m_name)) {}


			std::string make_name(const std::string& key) {
				return m_arrayFlag ? std::to_string(m_arrayIndex++) : key;
			}
			bool array() {
				return m_arrayFlag;
			}
			const std::string& name() {
				return m_name;
			}
			const std::string& path() {
				return m_path;
			}
		};

		static std::vector<context_t> m_stack;

		static std::tuple<std::string, std::string> path_name(const std::string& key) {
			//            path         name
			if (m_stack.empty()) {
				return {"", ""};
			} else {
				context_t&  parent = m_stack.back();
				std::string name   = parent.make_name(key);
				std::string path   = parent.path() + "/" + name;
				return {path, name};
			}
		}
		static void push(const context_t& newValue) {
			m_stack.push_back(newValue);
		}
		static void pop() {
			m_stack.pop_back();
		}
		static int level() {
			return (int)m_stack.size();
		}


		//
		// callback
		//
		void start() {
			m_stack.clear();
		}
		void stop() {
			//
		}
		void item (const std::string& key, const std::string& value) {
			const auto [path, name] = path_name(key);
			std::cout << path << " " << value << std::endl;
		}
		void enter(const std::string& key, const bool         isArray){
			const auto [path, name] = path_name(key);
			context_t my(isArray, path, name);
			push(my);

		}
		void leave() {
			pop();
		}
	};

	void dump_dot(std::istream& in);

}
}
