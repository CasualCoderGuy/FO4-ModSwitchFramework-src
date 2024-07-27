#pragma once
#include <unordered_map>

class IDDatabase
{
private:
	IDDatabase();
	uintptr_t _base;
	std::unordered_map<int, ptrdiff_t> id_database;

public:
	[[nodiscard]] static IDDatabase& get()
	{
		static IDDatabase singleton;
		return singleton;
	}

	[[nodiscard]] ptrdiff_t id2offset(int a_id) const
	{
		auto itAddr = id_database.find(a_id);
		if (itAddr != id_database.end())
			return itAddr->second;
		else
			return 0;
	}

	[[nodiscard]] ptrdiff_t id2addr(int a_id) const
	{
		auto itAddr = id_database.find(a_id);
		if (itAddr != id_database.end())
			return _base + itAddr->second;
		else
			return 0;
	}

	[[nodiscard]] uintptr_t base() const
	{
		return _base;
	}
};

class Offset
{
public:
	constexpr Offset() noexcept = default;

	explicit constexpr Offset(std::size_t a_offset) noexcept :
		_offset(a_offset)
	{}

	constexpr Offset& operator=(std::size_t a_offset) noexcept
	{
		_offset = a_offset;
		return *this;
	}

	[[nodiscard]] std::uintptr_t address() const { return base() + offset(); }
	[[nodiscard]] constexpr std::size_t offset() const noexcept { return _offset; }

private:
	[[nodiscard]] static std::uintptr_t base() { return IDDatabase::get().base(); }

	std::size_t _offset{ 0 };
};

class ID
{
public:
	constexpr ID() noexcept = default;

	explicit constexpr ID(std::uint64_t a_id, std::uint64_t a_id_ng = 0) noexcept :
		_id(a_id), _id_ng(a_id_ng)
	{}

	constexpr ID& operator=(std::uint64_t a_id) noexcept
	{
		_id = a_id;
		return *this;
	}

	[[nodiscard]] std::uintptr_t address() const { return base() + offset(); }
	[[nodiscard]] constexpr std::uint64_t id() const noexcept { return _id; }
	[[nodiscard]] std::size_t offset() const { return IDDatabase::get().id2offset(_id); }

private:
	[[nodiscard]] static std::uintptr_t base() { return IDDatabase::get().base(); }

	std::uint64_t _id{ static_cast<std::uint64_t>(-1) };
	std::uint64_t _id_ng{ static_cast<std::uint64_t>(-1) };
};