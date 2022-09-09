#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "SPRB/ENG.hpp"
#include "SPRB/TextureHashes.hpp"
#include "nn/fs.h"
#include "xxhash.h"
#include <string>
#include <algorithm>
#define Unicode std::char_traits<char16_t>

struct tmp {
	uint32_t fontsize;
	float X_Scale;
} RenderTextStruct;

struct sprbText {
	char* string;
	uint32_t byte_count;
} _sprbText;

uintptr_t TextRegionOffset = 0;

ptrdiff_t returnInstructionOffset(uintptr_t LR) {
	return LR - TextRegionOffset;
}

namespace SHIMAMON_DAT {
	int CardDescription = -1;
	int FighterDescription = -1;
	const char16_t* PlayerNameForAttack = nullptr;
}

uint64_t buffer[3*1024*1024];

uint64_t (*RenderText_original)(void* x0, tmp* RTS, void* x2, sprbText* TextStruct, void* x4, int x5, int x6, int x7, void* x8, void* x9, void* x10);
uint64_t RenderText_hook(void* x0, tmp* RTS, void* x2, sprbText* TextStruct, void* x4, int x5, int x6, int x7, void* x8, void* x9, void* x10) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

	switch(offsetItr) {
		//Shiroha Naruse
		case 0x18E474:
			RTS->X_Scale = 0.9;
			break;
		//Kamome Kimishima
		case 0x18E50C:
			RTS->X_Scale = 0.8;
			break;
		//Tsumugi Wenders
		case 0x18E55C:
			RTS->X_Scale = 0.8;
			break;
		//Shiki Kamiyama
		case 0x18E98C:
			RTS->X_Scale = 0.9;
			break;
		//Ryouichi Mitani
		case 0x18E9D8:
			RTS->X_Scale = 0.9;
			break;
		default:
			RTS->X_Scale = 1.0;
	}

	return RenderText_original(x0, RTS, x2, TextStruct, x4, x5, x6, x7, x8, x9, x10);
}

uint64_t (*CMD_LOG_original)(void* x0, void* x1, int voiceid, const char16_t* message);
uint64_t CMD_LOG_hook(void* x0, void* x1, int voiceid, const char16_t* message) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
	switch(offsetItr) {
		case 0x1F36D8 ... 0x1F3858: {
			auto itr = STRINGS::LOG_EF1.find(message);
			if (itr != STRINGS::LOG_EF1.end())
				message = itr->second;
			break;
		}
		case 0x1F3E58 ... 0x1F4020: {
			auto itr = STRINGS::LOG_EF6.find(message);
			if (itr != STRINGS::LOG_EF6.end())
				message = itr->second;
			break;
		}
		case 0x1F4050 ... 0x1F4230: {
			auto itr = STRINGS::LOG_EF7.find(message);
			if (itr != STRINGS::LOG_EF7.end())
				message = itr->second;
			break;
		}
	}
	return CMD_LOG_original(x0, x1, voiceid, message);
}

uint64_t (*boot_original)(void* unk0);
uint64_t boot_hook(void* unk0) {
	uintptr_t New_Warningptr = (uintptr_t)ENG::bootWarning;
	
	memcpy((void*)(TextRegionOffset + 0x566480), (void*)&New_Warningptr, 8);
	return boot_original(unk0);
}

uint64_t (*PrintDate_original)(void* unk0, const char* format, void* year, void* month, void* day, void* x5, void* x6, void* x7, void* x8);
uint64_t PrintDate_hook(void* unk0, const char* format, void* year, void* month, void* day, void* x5, void* x6, void* x7, void* x8){
	const char* old_dateformat = "%04d年%2d月%2d日";
	if (strncmp(old_dateformat, format, 19) == 0) {
		return PrintDate_original(unk0, &ENG::save_dateformat[0], day, month, year, x5, x6, x7, x8);
	}
	else
		return PrintDate_original(unk0, format, year, month, day, x5, x6, x7, x8);
}

uint64_t (*Parse_original)(const char* UTF8_string, void* unk1);
uint64_t Parse_hook(const char* UTF8_string, void* unk1)
{
	auto itr = STRINGS::COMMON.find(UTF8_string);
	if (itr != STRINGS::COMMON.end())
		UTF8_string = itr->second;
	return Parse_original(UTF8_string, unk1);
}

bool passShimapongDoublesNames(char16_t* buffer, size_t buffer_size, const char16_t* string) {
	memset((void*)buffer, 0, buffer_size*2);
	auto itr = ENG::SHIMAPONG::DOUBLES.find(string);
	if (itr != ENG::SHIMAPONG::DOUBLES.end()) {
		Unicode::copy(buffer, itr->second, Unicode::length(itr->second));
		return !true;
	}
	else {
		for (size_t i = 0; i < std::size(ENG::SHIMAPONG::PLAYERS); i++) {
			if (Unicode::compare(ENG::SHIMAPONG::PLAYERS[i].JPN, string, Unicode::length(ENG::SHIMAPONG::PLAYERS[i].JPN)))
				continue;
			Unicode::copy(buffer, ENG::SHIMAPONG::PLAYERS[i].ENG, Unicode::length(ENG::SHIMAPONG::PLAYERS[i].ENG));
			int buffer_cursor = Unicode::length(ENG::SHIMAPONG::PLAYERS[i].ENG);
			int string_cursor = Unicode::length(ENG::SHIMAPONG::PLAYERS[i].JPN);
			Unicode::copy(&buffer[buffer_cursor], &u"&"[0], 1);
			buffer_cursor++;
			string_cursor++;
			for (size_t x = 0; x < std::size(ENG::SHIMAPONG::PLAYERS); x++) {
				if (Unicode::compare(ENG::SHIMAPONG::PLAYERS[x].JPN, string+string_cursor, Unicode::length(ENG::SHIMAPONG::PLAYERS[x].JPN)))
					continue;
				Unicode::copy(&buffer[buffer_cursor], ENG::SHIMAPONG::PLAYERS[x].ENG, Unicode::length(ENG::SHIMAPONG::PLAYERS[x].ENG));
				return !true;
			}
		}
	}
	return !false;
}

uint64_t (*popup_original)(void* x0, int fontsize, const char16_t* string, int w3, int w4, int w5, int w6, int ABGR, int w8);
uint64_t popup_hook(void* x0, int fontsize, const char16_t* string, int w3, int w4, int w5, int w6, int ABGR, int w8)
{
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

	switch(offsetItr) {
		//Achievement popup
		case 0x1DC7DC: {
			if (!Unicode::compare(string, u"レコードを獲得しました！", Unicode::length(u"レコードを獲得しました！")))
				string = ENG::medalObtained;
			break;
		}
		//Table Tennis Tenzen dialogues
		case 0x3046E8: {
			auto itr = STRINGS::TenzenTableMatch.find(string);
			if (itr != STRINGS::TenzenTableMatch.end())
				string = itr->second;
			break;
		}
		//Table Tennis popup
		case 0x30F400: {
			auto itr = STRINGS::TennisTablePopup.find(string);
			if (itr != STRINGS::TennisTablePopup.end())
				string = itr->second;
			break;
		}
		//Summary after finishing table tennis match with Tenzen
		case 0x3195C0: {
			auto itr = STRINGS::TenzenTableSummary.find(string);
			if (itr != STRINGS::TenzenTableSummary.end())
				string = itr->second;
			break;
		}
		//Hack to avoid limitation of Shimamon Card Description Buffer (128 char16_t characters), otherwise segfault
		case 0x2D41F4: {
			if (SHIMAMON_DAT::CardDescription != -1) {
				string = ENG::SHIMAMON::SHIMAMONS[SHIMAMON_DAT::CardDescription].Description;
				SHIMAMON_DAT::CardDescription = -1;
			}
			break;
		}
		//Adjust size of Shimamon Fighters names on Map to not go out of bounds
		case 0x2DB378: {
			if (!Unicode::compare(ENG::Shizuku_alt[0], string, Unicode::length(ENG::Shizuku_alt[0])))
				fontsize = 18;
		}

		/*Shimapong texts in message box and pop up, they are concatenated via function at 0x21BF20 that is using u16string_view as source,
		and because it has predefined size and free space is checked BEFORE function is called via simple asm loop check,
		we cannot use it to pass our strings via it. We need to use this case that has already text merged and calculates size
		via passed string*/
		case 0x24A8E8: {
			//Check one exclusion that is related to calling Hairi name without actually using it, to fix not working Old-school racket text and maybe other texts
			if (SHIMAMON_DAT::PlayerNameForAttack == ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[0])
				if (Unicode::compare(ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[0], string, Unicode::length(ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[0])))
					SHIMAMON_DAT::PlayerNameForAttack = nullptr;
			//If we didn't get info that Player Name was checked before, check if it's an item at the beginning
			if (!SHIMAMON_DAT::PlayerNameForAttack) {
				for (size_t i = 0; i < std::size(ENG::SHIMAPONG::ItemsConStrings); i++) {
					static bool exit_bool = false;
					if (Unicode::compare(ENG::SHIMAPONG::ItemsConStrings[i].JPN, string, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN)))
						continue;
					char16_t buffer[64];
					memset((void*)&buffer[0], 0, 128);
					Unicode::copy(&buffer[0], ENG::SHIMAPONG::ItemsConStrings[i].ENG, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG));
					int buffer_cursor = Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG);
					int string_cursor = Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN);
					for (size_t x = 0; x < std::size(ENG::SHIMAPONG::AfterItemsConStrings); x++) {
						if (Unicode::compare(ENG::SHIMAPONG::AfterItemsConStrings[x].JPN, string+string_cursor, Unicode::length(ENG::SHIMAPONG::AfterItemsConStrings[x].JPN)))
							continue;
						Unicode::copy(&buffer[buffer_cursor], ENG::SHIMAPONG::AfterItemsConStrings[x].ENG, Unicode::length(ENG::SHIMAPONG::AfterItemsConStrings[x].ENG));
						string = &buffer[0];
						exit_bool = true;
						break;
					}
					if (exit_bool) {
						exit_bool = false;
						break;
					}
				}
				break;
			}
			/*Since all relevant concatenated attacks start with Player Name, we are checking if it's at the beginning of the passed string,
			if not break;*/
			size_t PlayerNameLength = Unicode::length(SHIMAMON_DAT::PlayerNameForAttack);
			if (Unicode::compare(SHIMAMON_DAT::PlayerNameForAttack, string, PlayerNameLength))
				break;
			char16_t buffer[128];
			memset((void*)&buffer[0], 0, 256);
			Unicode::copy(&buffer[0], SHIMAMON_DAT::PlayerNameForAttack, PlayerNameLength);

			/*Check string after name if we have on the list*/
			auto itr = ENG::SHIMAPONG::PlayerStatus.find(string+PlayerNameLength);
			if (itr != ENG::SHIMAPONG::PlayerStatus.end()) {
				Unicode::copy(&buffer[PlayerNameLength], itr->second, Unicode::length(itr->second));
				uint64_t ret = popup_original(x0, fontsize, (const char16_t*)&buffer[0], w3, w4, w5, w6, ABGR, w8);
				SHIMAMON_DAT::PlayerNameForAttack = nullptr;
				return ret;
			}

			Unicode::copy(&buffer[PlayerNameLength], u"の", 1);
			/*Check variant の, that comes in two possible concatenations*/
			if (!Unicode::compare((const char16_t*)&buffer[0], string, Unicode::length((const char16_t*)&buffer[0]))) {
				auto itr = ENG::SHIMAPONG::PlayerAttacks.find(string+PlayerNameLength);
				if (itr != ENG::SHIMAPONG::PlayerAttacks.end()) {
					Unicode::copy(&buffer[PlayerNameLength], itr->second, Unicode::length(itr->second));
					if (Unicode::length(&buffer[0]) > 59)
						fontsize = int(35.f * 59.f/float(Unicode::length(&buffer[0])));
					uint64_t ret = popup_original(x0, fontsize, (const char16_t*)&buffer[0], w3, w4, w5, w6, ABGR, w8);
					SHIMAMON_DAT::PlayerNameForAttack = nullptr;
					return ret;
				}

				int temp_string_cursor = PlayerNameLength+1;
				for (size_t i = 0; i < std::size(ENG::SHIMAPONG::ItemsConStrings); i++) {
					if (Unicode::compare(ENG::SHIMAPONG::ItemsConStrings[i].JPN, string+temp_string_cursor, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN)))
						continue;
					Unicode::copy(&buffer[PlayerNameLength], ENG::SHIMAPONG::no, Unicode::length(ENG::SHIMAPONG::no));
					int temp_buffer_length = PlayerNameLength+Unicode::length(ENG::SHIMAPONG::no);
					Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::ItemsConStrings[i].ENG, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG));
					temp_buffer_length += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG);
					temp_string_cursor += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN);
					for (size_t x = 0; x < std::size(ENG::SHIMAPONG::AfterItemsConStrings); x++) {
						if (Unicode::compare(ENG::SHIMAPONG::AfterItemsConStrings[x].JPN, string+temp_string_cursor, Unicode::length(ENG::SHIMAPONG::AfterItemsConStrings[x].JPN)))
							continue;
						Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::AfterItemsConStrings[x].ENG, Unicode::length(ENG::SHIMAPONG::AfterItemsConStrings[x].ENG));
						temp_buffer_length += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[x].ENG);
						temp_string_cursor += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[x].JPN);
						uint64_t ret = popup_original(x0, fontsize, (const char16_t*)&buffer[0], w3, w4, w5, w6, ABGR, w8);
						SHIMAMON_DAT::PlayerNameForAttack = nullptr;
						return ret;
					}
				}
			}

			Unicode::copy(&buffer[PlayerNameLength], u"は", 1);
			/*Check variant は*/
			if (!Unicode::compare((const char16_t*)&buffer[0], string, Unicode::length((const char16_t*)&buffer[0]))) {
				int temp_string_cursor = PlayerNameLength+1;
				for (size_t i = 0; i < std::size(ENG::SHIMAPONG::ItemsConStrings); i++) {
					if (Unicode::compare(ENG::SHIMAPONG::ItemsConStrings[i].JPN, string+temp_string_cursor, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN)))
						continue;
					temp_string_cursor += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN);
					for (size_t x = 0; x < std::size(ENG::SHIMAPONG::wa); x++) {
						if (Unicode::compare(ENG::SHIMAPONG::wa[x].JPN, string+temp_string_cursor, Unicode::length(ENG::SHIMAPONG::wa[x].JPN)))
							continue;
						Unicode::copy(&buffer[PlayerNameLength], ENG::SHIMAPONG::wa[x].ENG, Unicode::length(ENG::SHIMAPONG::wa[x].ENG));
						int temp_buffer_length = PlayerNameLength + Unicode::length(ENG::SHIMAPONG::wa[x].ENG);
						Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::ItemsConStrings[i].ENG, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG));
						temp_buffer_length += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG);
						Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::EndConString, Unicode::length(ENG::SHIMAPONG::EndConString));
						uint64_t ret = popup_original(x0, fontsize, (const char16_t*)&buffer[0], w3, w4, w5, w6, ABGR, w8);
						SHIMAMON_DAT::PlayerNameForAttack = nullptr;
						return ret;
					}

				}
			}

			Unicode::copy(&buffer[PlayerNameLength], u"から", 2);
			/*Check variant から*/
			if (!Unicode::compare((const char16_t*)&buffer[0], string, Unicode::length((const char16_t*)&buffer[0]))) {
				int temp_string_cursor = PlayerNameLength+2;
				for (size_t i = 0; i < std::size(ENG::SHIMAPONG::ItemsConStrings); i++) {
					if (Unicode::compare(ENG::SHIMAPONG::ItemsConStrings[i].JPN, string+temp_string_cursor, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].JPN)))
						continue;
					Unicode::copy(&buffer[PlayerNameLength], ENG::SHIMAPONG::kara, Unicode::length(ENG::SHIMAPONG::kara));
					int temp_buffer_length = PlayerNameLength + Unicode::length(ENG::SHIMAPONG::kara);
					Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::ItemsConStrings[i].ENG, Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG));
					temp_buffer_length += Unicode::length(ENG::SHIMAPONG::ItemsConStrings[i].ENG);
					Unicode::copy(&buffer[temp_buffer_length], ENG::SHIMAPONG::EndConString, Unicode::length(ENG::SHIMAPONG::EndConString));
					uint64_t ret = popup_original(x0, fontsize, (const char16_t*)&buffer[0], w3, w4, w5, w6, ABGR, w8);
					SHIMAMON_DAT::PlayerNameForAttack = nullptr;
					return ret;
				}
			}

			break;
		}
			
		case 0x299100:
		case 0x300AB0: {
			auto itr = ENG::SHIMAPONG::FinishShot.find(string);
			if (itr != ENG::SHIMAPONG::FinishShot.end())
				string = itr->second;
			break;
		}

		//Check if we are dealing with Shimamon Fight Ryouichi Special Move to adjust font size so we can fit longer strings
		case 0x2474B4: {
			if (Unicode::find(string, Unicode::length(string), u"\n"[0]))
				fontsize = 24;
			break;
		}

		/*Hook rendering Doubles Team names at the beginning of match here instead of function that calls string
		because text is later copied to char16_t buffer[32], which is not enough for our case.
		Also adjust font size for longer strings*/
		case 0x29B93C: {
			char16_t buffer[64];
			if (!passShimapongDoublesNames(&buffer[0], 64, string))
				string = &buffer[0];
			if (Unicode::length(string) > 27)
				fontsize = int((float)fontsize * (27.f/(float)Unicode::length(string)));
			break;
		}

		/*Hook rendering Doubles Team name at the end of match here instead of function that calls string
		because text is later copied to char16_t buffer[32], which is not enough for our case.*/
		case 0x298F04: {
			char16_t buffer[64];
			if (!passShimapongDoublesNames(&buffer[0], 64, string))
				string = &buffer[0];
			break;
		}

		/*Hook rendering Doubles Team name at Shimapong ending
		because text is later copied to char16_t buffer[32], which is not enough for our case.*/
		case 0x300510: {
			char16_t buffer[64];
			Unicode::copy(&buffer[0], &u"vs "[0], 3);
			int buffer_cursor = 3;
			int string_cursor = 3;
			if (!passShimapongDoublesNames(&buffer[buffer_cursor], 61, string+string_cursor))
				string = &buffer[0];
			break;
		}
	}
	return popup_original(x0, fontsize, string, w3, w4, w5, w6, ABGR, w8);
}

const char16_t* (*get_MedalName_original)(int medalID);
const char16_t* get_MedalName_hook(int medalID) {
	if (medalID == 142) {
		const char16_t* original_string = get_MedalName_original(medalID);
		if (!Unicode::compare(u"島モンファイトで満月を喰らいし乳房に勝利した", original_string, Unicode::length(u"島モンファイトで満月を喰らいし乳房に勝利した")))
			return ENG::MEDALS[250];
	}
	return ENG::MEDALS[medalID];
}

const char16_t* (*get_TitleName_original)(int titleID);
const char16_t* get_TitleName_hook(int titleID) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

	switch(offsetItr) {
		case 0x2EDBBC:
		case 0x2D6E6C:
		case 0x2B9F98:
			return ENG::CLEAN_TITLES[titleID-1];
			break;
	}
	return ENG::TITLES[titleID-1];
}

const char16_t* (*get_ItemName_original)(int itemID);
const char16_t* get_ItemName_hook(int itemID) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

	switch(offsetItr) {
		case 0x326AA8: {
			const char16_t* original_string = get_ItemName_original(itemID);
			if (!original_string || (original_string[0] == 0))
				break;
			auto itr = STRINGS::ITEMS_POPUP.find(original_string);
			if (itr != STRINGS::ITEMS_POPUP.end())
				return itr->second;
			break;
		}
	}
	return get_ItemName_original(itemID);
}

uint64_t (*CMD_LOG2_original)(void* x0, const char16_t* string, int w2);
uint64_t CMD_LOG2_hook(void* x0, const char16_t* string, int w2) {
	if (string && string[0] > 0) {
		auto itr = STRINGS::LOG2.find(string);
    	if (itr != STRINGS::LOG2.end())
			string = itr->second;
	}
	return CMD_LOG2_original(x0, string, w2);
}

const char* compareHashes(uint64_t hashToCompare) {
	for(size_t i = 0; i < std::size(TEXTURE_HASHES); i++)
		if (TEXTURE_HASHES[i].hash == hashToCompare) 
			return TEXTURE_HASHES[i].filepath;
	return nullptr;
}

bool CZ0_bypass = false;

uint64_t (*registerTexture_original)(void* x0, int w1, void* x2, void* texture_ptr, int texture_size, int w5, int w6);
uint64_t registerTexture_hook(void* x0, int w1, void* x2, void* texture_ptr, int texture_size, int w5, int w6) {
	int64_t size = 0;
	u64 filesize = 0;
	XXH64_hash_t hash_output = XXH3_64bits(texture_ptr, texture_size);
	const char* index = compareHashes(hash_output);
	if (index != nullptr) {
		//Texture swapping

		nn::fs::FileHandle filehandle;
		char filepath[128] = "";
		snprintf(&filepath[0], 128, "files:/%s.dat", index);
		if(R_FAILED(nn::fs::OpenFile(&filehandle, filepath, nn::fs::OpenMode_Read)))
			return registerTexture_original(x0, w1, x2, texture_ptr, texture_size, w5, w6);

		nn::fs::GetFileSize(&size, filehandle);
		nn::fs::ReadFile(&filesize, filehandle, 0, &buffer, size);
		nn::fs::CloseFile(filehandle);

		//Bypass is needed to pass CZ0 textures through function that is not dedicated for it.
		if(!strncmp((const char*)&buffer, "CZ0", 3))
			CZ0_bypass = true;
		uint64_t ptr_ret = registerTexture_original(x0, w1, x2, buffer, filesize, w5, w6);
		CZ0_bypass = false;

		return ptr_ret;
	}
	else return registerTexture_original(x0, w1, x2, texture_ptr, texture_size, w5, w6);
}

int (*strcmp_original) (const char *p1, const char *p2);
int strcmp_hook (const char *p1, const char *p2) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

	if (offsetItr != 0x14F030 || !CZ0_bypass)
		return strcmp_original(p1, p2);
	else return 1;
}

int (*TextFormatSP_original)(void* x0, int w1, const char16_t* string, int w3);
int TextFormatSP_hook(void* x0, int w1, const char16_t* string, int w3) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
	switch(offsetItr) {
		case 0x1F7224:
			string = ENG::SHIMAMON::Deck_default_names[0];
			break;
		case 0x1F7238:
			string = ENG::SHIMAMON::Deck_default_names[1];
			break;
		case 0x1F724C:
			string = ENG::SHIMAMON::Deck_default_names[2];
			break;
	}
	return TextFormatSP_original(x0, w1, string, w3);
}

namespace SHIMAMON_FUN {

	int (*sprintf16_original)(void* buffer, const char16_t* format, void* x2, void* x3, void* x4, void* x5, void* x6, void* x7, void* x8);
	int sprintf16_hook(void* buffer, const char16_t* format, void* x2, void* x3, void* x4, void* x5, void* x6, void* x7, void* x8) {
		auto itr = ENG::SHIMAMON::sprintf16_formats.find(format);
		if (itr != ENG::SHIMAMON::sprintf16_formats.end())
			format = itr->second;
		return sprintf16_original(buffer, format, x2, x3, x4, x5, x6, x7, x8);
	}

	void (*DeckChangeName_original)(void* x0, const char* string, void* x2, int w3);
	void DeckChangeName_hook(void* x0, const char* string, void* x2, int w3) {
		ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
		if (offsetItr == 0x2B31B8)
			string = ENG::SHIMAMON::Deck_name_prompt;
		return DeckChangeName_original(x0, string, x2, w3);
	}

	const char16_t* (*get_FighterName_original)(int titleID);
	const char16_t* get_FighterName_hook(int titleID) {
		const char16_t* original_string = get_FighterName_original(titleID);
		//We need to hook this specifically because game reference it only directly instead of using array
		if (!Unicode::compare(u"満月を喰らいし乳房", original_string, Unicode::length(u"満月を喰らいし乳房")))
			return ENG::Shizuku_alt[0];
		return original_string;
	}

	const char16_t* (*get_FighterDescription_original)(int itemID);
	const char16_t* get_FighterDescription_hook(int itemID) {
		ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

		switch(offsetItr) {
			case 0x2EE218: {
				if (itemID > 0) {
					SHIMAMON_DAT::FighterDescription = itemID - 1;
					break;
				}
			}
			default: {
				SHIMAMON_DAT::FighterDescription = -1;
			}
		}
		return get_FighterDescription_original(itemID);
	}

	const char16_t* (*get_CardDescription_original)(int itemID);
	const char16_t* get_CardDescription_hook(int itemID) {
		ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));

		switch(offsetItr) {
			case 0x2D4074: {
				if (itemID > 0) {
					SHIMAMON_DAT::CardDescription = itemID - 1;
					break;
				}
			}
			default: {
				SHIMAMON_DAT::CardDescription = -1;
			}
		}
		return get_CardDescription_original(itemID);
	}

}

int (*set_ClassTextProperties_original)(void* x0, int w1, int w2, int w3, int w4, int fontsize, const char16_t* string);
int set_ClassTextProperties_hook(void* x0, int w1, int w2, int w3, int w4, int fontsize, const char16_t* string) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
	switch(offsetItr) {
		/*Control font size of Shimamon Fighters titles in animation
		at the start of fight since text window is wider than actual texture,
		so line break is not applied where it should*/
		case 0x2EE290:
			if (SHIMAMON_DAT::FighterDescription > 0) {
				int temp_fontsize = ENG::SHIMAMON::FIGHTERS[SHIMAMON_DAT::FighterDescription].Description_fontsize;
				if (temp_fontsize > -1)
					fontsize = temp_fontsize;
			}
	}
	return set_ClassTextProperties_original(x0, w1, w2, w3, w4, fontsize, string);
}

namespace SHIMAPONG_FUN {

	const char16_t* (*get_PlayerName_original)(int playerID);
	const char16_t* get_PlayerName_hook(int playerID) {
		ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
		switch(offsetItr) {
			case 0x21743C:
			case 0x217604:
				return get_PlayerName_original(playerID);
		}
		playerID -= 1;
		switch(playerID) {
			case 11: {
				const char16_t* original_string = get_PlayerName_original(playerID+1);
				if (!Unicode::compare(u"強面の老人", original_string, Unicode::length(u"強面の老人")))
					return ENG::SHIMAPONG::PLAYERS[17].ENG;
				else return ENG::SHIMAPONG::PLAYERS[11].ENG;
			}
			case 10: {
				const char16_t* original_string = get_PlayerName_original(playerID+1);
				if (!Unicode::compare(u"満月を喰らいし乳房さん", original_string, Unicode::length(u"満月を喰らいし乳房さん")))
					return ENG::Shizuku_alt[0];
				else return ENG::SHIMAPONG::PLAYERS[10].ENG;
			}
		}
		
		return ENG::SHIMAPONG::PLAYERS[playerID].ENG;
	}

	const char16_t* (*get_PlayerNameForAttack_original)(int playerID);
	const char16_t* get_PlayerNameForAttack_hook(int playerID) {
		playerID -= 1;
		switch(playerID) {
			case 11: {
				const char16_t* original_string = get_PlayerNameForAttack_original(playerID+1);
				if (!Unicode::compare(u"強面の老人", original_string, Unicode::length(u"強面の老人"))) {
					SHIMAMON_DAT::PlayerNameForAttack = ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[17];
					return ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[17];
				}
				else {
					SHIMAMON_DAT::PlayerNameForAttack = ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[11];
					return ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[11];
				}
				break;
			}
			case 10: {
				const char16_t* original_string = get_PlayerNameForAttack_original(playerID+1);
				if (!Unicode::compare(u"満月を喰らいし乳房さん", original_string, Unicode::length(u"満月を喰らいし乳房さん"))) {
					SHIMAMON_DAT::PlayerNameForAttack = ENG::Shizuku_alt[0];
					return ENG::Shizuku_alt[0];
				}
				else if (!Unicode::compare(u"水織さん", original_string, Unicode::length(u"水織さん"))) {
					SHIMAMON_DAT::PlayerNameForAttack = ENG::Shizuku_alt[1];
					return ENG::Shizuku_alt[1];
				}
					
				else {
					SHIMAMON_DAT::PlayerNameForAttack = ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[10];
					return ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[10];
				}
				break;
			}
		}
		SHIMAMON_DAT::PlayerNameForAttack = ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[playerID];
		return ENG::SHIMAPONG::PLAYERS_FOR_ATTACK[playerID];
	}

}

void (*u8StringPass_original)(void* x0, int w1, const char* string);
void u8StringPass_hook(void* x0, int w1, const char* string) {
	ptrdiff_t offsetItr = returnInstructionOffset((uintptr_t)__builtin_return_address(0));
	switch(offsetItr) {
		case 0x2F934C: {
			auto itr = ENG::SHIMAPONG::DifficultyPopup.find(string);
			if (itr != ENG::SHIMAPONG::DifficultyPopup.end())
				string = itr->second;
			break;
		}
		case 0x231A0C: {
			auto itr = ENG::SHIMAPONG::FreeMatchPopup.find(string);
			if (itr != ENG::SHIMAPONG::FreeMatchPopup.end())
				string = itr->second;
			break;
		}
	}
	return u8StringPass_original(x0, w1, string);
}

uint64_t (*initOST_original)();
uint64_t initOST_hook() {
	uint64_t ret = initOST_original();
	for (size_t i = 0; i < std::size(MUSIC::titleOffsets); i++) {
		const char** string = (const char**)(TextRegionOffset + MUSIC::titleOffsets[i]);
		*string = MUSIC::Titles[i];
	}
	return ret;
}

void SPRB_main()
{
	TextRegionOffset = (uintptr_t)skyline::utils::getRegionAddress(skyline::utils::region::Text);
	//UI Text
	A64HookFunction((void*)(TextRegionOffset + 0x1DB0), reinterpret_cast<void*>(Parse_hook), (void**)&Parse_original);
	A64HookFunction((void*)(TextRegionOffset + 0x10DCA0), reinterpret_cast<void*>(RenderText_hook), (void**)&RenderText_original);

	//Print Date
	A64HookFunction((void*)(TextRegionOffset + 0x5EB0), reinterpret_cast<void*>(PrintDate_hook), (void**)&PrintDate_original);

	//Boot
	A64HookFunction((void*)(TextRegionOffset + 0x1787C0), reinterpret_cast<void*>(boot_hook), (void**)&boot_original);

	//LOG function
	A64HookFunction((void*)(TextRegionOffset + 0x69C20), reinterpret_cast<void*>(CMD_LOG_hook), (void**)&CMD_LOG_original);

	//Special Text
	A64HookFunction((void*)(TextRegionOffset + 0x4A8A0), reinterpret_cast<void*>(popup_hook), (void**)&popup_original);

	//Get Medal Name
	A64HookFunction((void*)(TextRegionOffset + 0x1FE600), reinterpret_cast<void*>(get_MedalName_hook), (void**)&get_MedalName_original);

	//Get Title Name
	A64HookFunction((void*)(TextRegionOffset + 0x1F90F0), reinterpret_cast<void*>(get_TitleName_hook), (void**)&get_TitleName_original);

	//LOG function 2
	A64HookFunction((void*)(TextRegionOffset + 0x767C0), reinterpret_cast<void*>(CMD_LOG2_hook), (void**)&CMD_LOG2_original);

	//Get Item Name
	A64HookFunction((void*)(TextRegionOffset + 0x1F97A0), reinterpret_cast<void*>(get_ItemName_hook), (void**)&get_ItemName_original);

	//TextureRegister
	A64HookFunction((void*)(TextRegionOffset + 0x14EEE0), reinterpret_cast<void*>(registerTexture_hook), (void**)&registerTexture_original);

	A64HookFunction((void**)&strcmp, reinterpret_cast<void*>(strcmp_hook), (void**)&strcmp_original);

	//Shimamon custom sprintf UTF-16
	A64HookFunction((void*)(TextRegionOffset + 0x22CE0), reinterpret_cast<void*>(SHIMAMON_FUN::sprintf16_hook), (void**)&SHIMAMON_FUN::sprintf16_original);

	//Hook Deck change name keyboard prompt
	A64HookFunction((void*)(TextRegionOffset + 0x168690), reinterpret_cast<void*>(SHIMAMON_FUN::DeckChangeName_hook), (void**)&SHIMAMON_FUN::DeckChangeName_original);

	//Hook some special function that is doing including other things also formatting of default deck names
	A64HookFunction((void*)(TextRegionOffset + 0x893B0), reinterpret_cast<void*>(TextFormatSP_hook), (void**)&TextFormatSP_original);

	//Get Shimamon Fighter Name
	A64HookFunction((void*)(TextRegionOffset + 0x1F9820), reinterpret_cast<void*>(SHIMAMON_FUN::get_FighterName_hook), (void**)&SHIMAMON_FUN::get_FighterName_original);

	//Get Shimamon Fighter Description
	A64HookFunction((void*)(TextRegionOffset + 0x1F9080), reinterpret_cast<void*>(SHIMAMON_FUN::get_FighterDescription_hook), (void**)&SHIMAMON_FUN::get_FighterDescription_original);

	//Get Shimamon Card Description Name
	A64HookFunction((void*)(TextRegionOffset + 0x1F9710), reinterpret_cast<void*>(SHIMAMON_FUN::get_CardDescription_hook), (void**)&SHIMAMON_FUN::get_CardDescription_original);

	//Render class text
	A64HookFunction((void*)(TextRegionOffset + 0x202DA0), reinterpret_cast<void*>(set_ClassTextProperties_hook), (void**)&set_ClassTextProperties_original);

	//Get Shimapong Player Name
	A64HookFunction((void*)(TextRegionOffset + 0x216F80), reinterpret_cast<void*>(SHIMAPONG_FUN::get_PlayerName_hook), (void**)&SHIMAPONG_FUN::get_PlayerName_original);

	//Get Shimapong Player Name For Attack
	A64HookFunction((void*)(TextRegionOffset + 0x217140), reinterpret_cast<void*>(SHIMAPONG_FUN::get_PlayerNameForAttack_hook), (void**)&SHIMAPONG_FUN::get_PlayerNameForAttack_original);

	//Some UTF-8 function that is used among others to show menu popups in Shimamon Fight map
	A64HookFunction((void*)(TextRegionOffset + 0xB1D50), reinterpret_cast<void*>(u8StringPass_hook), (void**)&u8StringPass_original);

	//Init music informations, titles included
	A64HookFunction((void*)(TextRegionOffset + 0x17C1C0), reinterpret_cast<void*>(initOST_hook), (void**)&initOST_original);

	/////////////////////////////////////////////////////////////////////////
	uint8_t NOP[4] = {0x1f, 0x20, 0x03, 0xD5}; //NOP

	//Patch Title popup instructions to not add second part of text via memory manipulation
	sky_memcpy((void*)(TextRegionOffset+0x326760), &NOP, 4);
	sky_memcpy((void*)(TextRegionOffset+0x326768), &NOP, 4);
	sky_memcpy((void*)(TextRegionOffset+0x32676C), &NOP, 4);
	sky_memcpy((void*)(TextRegionOffset+0x326AA0), &NOP, 4);
	sky_memcpy((void*)(TextRegionOffset+0x326B0C), &NOP, 4);
	struct ShimamonRest {
		uint64_t ID;
		char16_t* Name;
		char reserved[8];
		char16_t* Description;
	};

	//Replace pointers for Shimamon baits
	ShimamonRest* SHIMAMON_ITEMS_POINTER = (ShimamonRest*)(TextRegionOffset+0x635CD0);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::ITEMS); i++) {
		SHIMAMON_ITEMS_POINTER[i].Name = (char16_t*)ENG::SHIMAMON::ITEMS[i].Name;
		SHIMAMON_ITEMS_POINTER[i].Description = (char16_t*)ENG::SHIMAMON::ITEMS[i].Description;
	}

	//Replace pointers for Shimamon locations
	ShimamonRest* SHIMAMON_BAIT_LOCATIONS_POINTER = (ShimamonRest*)(TextRegionOffset+0x635F70);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::BAIT_LOCATIONS); i++) {
		SHIMAMON_BAIT_LOCATIONS_POINTER[i].Name = (char16_t*)ENG::SHIMAMON::BAIT_LOCATIONS[i].Name;
	}

	char16_t** SHIMAMON_ATTRIBUTES_POINTER = (char16_t**)(TextRegionOffset+0x639168);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::ATTRIBUTES); i++) {
		SHIMAMON_ATTRIBUTES_POINTER[i] = (char16_t*)ENG::SHIMAMON::ATTRIBUTES[i];
	}
	struct ShimamonCards {
		uint64_t ID;
		char16_t* Name;
		char reserved[0x18];
		char16_t* Description;
		char reserved2[8];
	};

	//Replace pointers for Shimamon names and descriptions
	ShimamonCards* SHIMAMON_CARDS_ARRAY = (ShimamonCards*)(TextRegionOffset+0x634628);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::SHIMAMONS); i++) {
		SHIMAMON_CARDS_ARRAY[i].Name = (char16_t*)ENG::SHIMAMON::SHIMAMONS[i].Name;
		/*We can't pass pointers for description since function at 0x2D39C0 
		is copying it to char16_t buffer[128] in some struct,
		so for longer than ~128 characters strings we are getting segfault.
		It was mitigated by using popup_hook() at case 0x2D41F4 in pair with get_ShimamonCardDescription_hook()
		to avoid needless list search*/
	}
	
	struct ShimamonFighters {
		uint64_t ID;
		char16_t* Name;
		char reserved[0x48];
		char16_t* Description;
		char16_t* AlterName;
		char reserved2[0x10];
	};

	//Replace pointers for Shimamon Fighters
	ShimamonFighters* SHIMAMON_FIGHTERS_ARRAY_RB = (ShimamonFighters*)(TextRegionOffset+0x6372F8);
	//ShimamonFighters* SHIMAMON_FIGHTERS_ARRAY = (ShimamonFighters*)(TextRegionOffset+0x636128); ///Array for game before expansion came out - it lacks Shiki
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::FIGHTERS); i++) {
		SHIMAMON_FIGHTERS_ARRAY_RB[i].Name = (char16_t*)ENG::SHIMAMON::FIGHTERS[i].Name;
		if (ENG::SHIMAMON::FIGHTERS[i].Description != nullptr) {
			SHIMAMON_FIGHTERS_ARRAY_RB[i].Description = (char16_t*)ENG::SHIMAMON::FIGHTERS[i].Description;
		}
		if (ENG::SHIMAMON::FIGHTERS[i].AlterName != nullptr) {
			SHIMAMON_FIGHTERS_ARRAY_RB[i].AlterName = (char16_t*)ENG::SHIMAMON::FIGHTERS[i].AlterName;
		}
	}

	struct ShimamonFightersLocations {
		uint64_t ID;
		char16_t* Name;
		char reserved[0x10];
	};

	ShimamonFightersLocations* SHIMAMON_FIGHTERS_LOCATIONS_POINTER = (ShimamonFightersLocations*)(TextRegionOffset+0x638B78);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::FIGHTERS_LOCATIONS); i++) {
		SHIMAMON_FIGHTERS_LOCATIONS_POINTER[i].Name = (char16_t*)ENG::SHIMAMON::FIGHTERS_LOCATIONS[i];
	}

	struct ShimamonAttacks {
		uint64_t ID;
		char16_t* Name;
		char reserved[0x10];
		char16_t* Description;
	};

	ShimamonAttacks* SHIMAMON_ATTACKS_POINTER = (ShimamonAttacks*)(TextRegionOffset+0x6384F0);
	for (size_t i = 0; i < std::size(ENG::SHIMAMON::ATTACKS); i++) {
		SHIMAMON_ATTACKS_POINTER[i].Name = (char16_t*)ENG::SHIMAMON::ATTACKS[i];
	}

	struct ShimapongMessages {
		uint64_t ID1;
		uint64_t ID2;
		char16_t* Message;
	};

	ShimapongMessages* SHIMAPONG_MESSAGES_POINTER = (ShimapongMessages*)(TextRegionOffset+0x639ED8);
	for (size_t i = 0; i < std::size(ENG::SHIMAPONG::MESSAGES); i++) {
		SHIMAPONG_MESSAGES_POINTER[i].Message = (char16_t*)ENG::SHIMAPONG::MESSAGES[i];
	}

	char16_t** SHIMAPONG_TABLEPOPUPS1_POINTER = (char16_t**)(TextRegionOffset+0x639C58);
	for (size_t i = 0; i < std::size(ENG::SHIMAPONG::TableStandardPopups1); i++) {
		SHIMAPONG_TABLEPOPUPS1_POINTER[i] = (char16_t*)ENG::SHIMAPONG::TableStandardPopups1[i];
	}

	char16_t** SHIMAPONG_TABLEPOPUPS2_POINTER = (char16_t**)(TextRegionOffset+0x639CC8);
	for (size_t i = 0; i < std::size(ENG::SHIMAPONG::TableStandardPopups2); i++) {
		SHIMAPONG_TABLEPOPUPS2_POINTER[i] = (char16_t*)ENG::SHIMAPONG::TableStandardPopups2[i];
	}

	char16_t** SHIMAPONG_TABLEPOPUPS3_POINTER = (char16_t**)(TextRegionOffset+0x639E00);
	for (size_t i = 0; i < std::size(ENG::SHIMAPONG::TableStandardPopups3); i++) {
		SHIMAPONG_TABLEPOPUPS3_POINTER[i] = (char16_t*)ENG::SHIMAPONG::TableStandardPopups3[i];
	}

	char16_t** SHIMAPONG_TABLEPOPUPS4_POINTER = (char16_t**)(TextRegionOffset+0x639D38);
	for (size_t i = 0; i < std::size(ENG::SHIMAPONG::TableStandardPopups4); i++) {
		SHIMAPONG_TABLEPOPUPS4_POINTER[i] = (char16_t*)ENG::SHIMAPONG::TableStandardPopups4[i];
	}

}
