#pragma once
#include <unordered_map>

uint64_t CheckHooks();
uint32_t CheckHookTargets();
uint32_t CheckHookCopies();

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

	explicit constexpr Offset(std::size_t a_offset, std::size_t a_offset_ng = 0) noexcept :
#ifndef NEXTGEN
		_offset(a_offset)
#else
		_offset((a_offset_ng == 0) ? a_offset : a_offset_ng)
#endif
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
#ifndef NEXTGEN
		_id(a_id)
#else
		_id((a_id_ng == 0) ? a_id : a_id_ng)
#endif
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
};