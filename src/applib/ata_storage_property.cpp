/******************************************************************************
License: GNU General Public License v3.0 only
Copyright:
	(C) 2008 - 2021 Alexander Shaduri <ashaduri@gmail.com>
******************************************************************************/
/// \file
/// \author Alexander Shaduri
/// \ingroup applib
/// \weakgroup applib
/// @{

#include "local_glibmm.h"
#include <map>
#include <ostream>  // not iosfwd - it doesn't work
#include <sstream>
#include <iomanip>
#include <locale>

#include "hz/string_num.h"  // number_to_string
#include "hz/stream_cast.h"  // stream_cast<>
#include "hz/format_unit.h"  // format_time_length
#include "hz/string_algo.h"  // string_join
#include "hz/string_num.h"  // number_to_string

#include "ata_storage_property.h"



std::ostream& operator<< (std::ostream& os, const AtaStorageCapability& p)
{
	os
			// << p.name << ": "
			<< p.flag_value;
	for (auto&& v : p.strvalues) {
		os << "\n\t" << v;
	}
	return os;
}



std::string AtaStorageAttribute::get_attr_type_name(AtaStorageAttribute::AttributeType type)
{
	static const std::unordered_map<AttributeType, std::string> m {
			{AttributeType::unknown, "[unknown]"},
			{AttributeType::prefail, "pre-failure"},
			{AttributeType::old_age, "old age"},
	};
	if (auto iter = m.find(type); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



std::string AtaStorageAttribute::get_update_type_name(AtaStorageAttribute::UpdateType type)
{
	static const std::unordered_map<UpdateType, std::string> m {
			{UpdateType::unknown, "[unknown]"},
			{UpdateType::always, "continuously"},
			{UpdateType::offline, "on offline data collect."},
	};
	if (auto iter = m.find(type); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



std::string AtaStorageAttribute::get_fail_time_name(AtaStorageAttribute::FailTime type)
{
	static const std::unordered_map<FailTime, std::string> m {
			{FailTime::unknown, "[unknown]"},
			{FailTime::none, "never"},
			{FailTime::past, "in the past"},
			{FailTime::now, "now"},
			{FailTime::NA, "not available"}, // nvme
	};
	if (auto iter = m.find(type); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



std::string AtaStorageAttribute::format_raw_value() const
{
	// If it's fully a number, format it with commas
	if (hz::number_to_string_nolocale(raw_value_int) == raw_value) {
		std::stringstream ss;
		try {
			ss.imbue(std::locale(""));
		}
		catch (const std::runtime_error& e) {
			// something is wrong with system locale, can't do anything here.
		}
		ss << std::fixed << raw_value_int;
		return ss.str();
	}
	return raw_value;
}



std::ostream& operator<< (std::ostream& os, const AtaStorageAttribute& p)
{
//	os << p.name << ": "
	if (p.value.has_value()) {
		os << static_cast<int>(p.value.value());
	} else {
		os << "-";
	}
	os << " (" << p.format_raw_value() << ")";
	return os;
}



bool AtaStorageStatistic::is_normalized() const
{
	return flags.find('N') != std::string::npos;
}



std::string AtaStorageStatistic::format_value() const
{
	// If it's fully a number, format it with commas
	if (hz::number_to_string_nolocale(value_int) == value) {
		std::stringstream ss;
		try {
			ss.imbue(std::locale(""));
		}
		catch (const std::runtime_error& e) {
			// something is wrong with system locale, can't do anything here.
		}
		ss << std::fixed << value_int;
		return ss.str();
	}
	return value;
}



std::ostream& operator<<(std::ostream& os, const AtaStorageStatistic& p)
{
	os << p.value;
	return os;
}



std::string AtaStorageErrorBlock::get_displayable_error_types(const std::vector<std::string>& types)
{
	static const std::map<std::string, std::string> m = {
		{"ABRT", _("Command aborted")},
		{"AMNF", _("Address mark not found")},
		{"CCTO", _("Command completion timed out")},
		{"EOM", _("End of media")},
		{"ICRC", _("Interface CRC error")},
		{"IDNF", _("Identity not found")},
		{"ILI", _("(Packet command-set specific)")},
		{"MC", _("Media changed")},
		{"MCR", _("Media change request")},
		{"NM", _("No media")},
		{"obs", _("Obsolete")},
		{"TK0NF", _("Track 0 not found")},
		{"UNC", _("Uncorrectable error in data")},
		{"WP", _("Media is write protected")},
	};

	std::vector<std::string> sv;
	for (const auto& type : types) {
		if (m.find(type) != m.end()) {
			sv.push_back(m.at(type));
		} else {
			std::string name = _("Uknown type");
			if (!type.empty()) {
				name = Glib::ustring::compose(_("Uknown type: %1"), type);
			}
			sv.push_back(name);
		}
	}

	return hz::string_join(sv, _(", "));
}



WarningLevel AtaStorageErrorBlock::get_warning_level_for_error_type(const std::string& type)
{
	static const std::map<std::string, WarningLevel> m = {
		{"ABRT", WarningLevel::none},
		{"AMNF", WarningLevel::alert},
		{"CCTO", WarningLevel::warning},
		{"EOM", WarningLevel::warning},
		{"ICRC", WarningLevel::warning},
		{"IDNF", WarningLevel::alert},
		{"ILI", WarningLevel::notice},
		{"MC", WarningLevel::none},
		{"MCR", WarningLevel::none},
		{"NM", WarningLevel::none},
		{"obs", WarningLevel::none},
		{"TK0NF", WarningLevel::alert},
		{"UNC", WarningLevel::alert},
		{"WP", WarningLevel::none},
	};

	if (m.find(type) != m.end()) {
		return m.at(type);
	}
	return WarningLevel::none;  // unknown error
}



std::string AtaStorageErrorBlock::format_lifetime_hours() const
{
	std::stringstream ss;
	try {
		ss.imbue(std::locale(""));
	}
	catch (const std::runtime_error& e) {
		// something is wrong with system locale, can't do anything here.
	}
	ss << std::fixed << lifetime_hours;
	return ss.str();
}



std::ostream& operator<< (std::ostream& os, const AtaStorageErrorBlock& b)
{
	os << "Error number " << b.error_num << ": "
		<< hz::string_join(b.reported_types, ", ")
		<< " [" << AtaStorageErrorBlock::get_displayable_error_types(b.reported_types) << "]";
	return os;
}



std::string AtaStorageSelftestEntry::get_status_displayable_name(AtaStorageSelftestEntry::Status s)
{
	static const std::unordered_map<Status, std::string> m {
			{Status::unknown, "[unknown]"},
			{Status::completed_no_error, "Completed without error"},
			{Status::aborted_by_host, "Manually aborted"},
			{Status::interrupted, "Interrupted (host reset)"},
			{Status::fatal_or_unknown, "Fatal or unknown error"},
			{Status::compl_unknown_failure, "Completed with unknown failure"},
			{Status::compl_electrical_failure, "Completed with electrical failure"},
			{Status::compl_servo_failure, "Completed with servo/seek failure"},
			{Status::compl_read_failure, "Completed with read failure"},
			{Status::compl_handling_damage, "Completed: handling damage"},
			{Status::in_progress, "In progress"},
			{Status::reserved, "Unknown / reserved state"},
	};
	if (auto iter = m.find(s); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



AtaStorageSelftestEntry::StatusSeverity AtaStorageSelftestEntry::get_status_severity(AtaStorageSelftestEntry::Status s)
{
	static const std::unordered_map<Status, StatusSeverity> m {
			{Status::unknown, StatusSeverity::none},
			{Status::completed_no_error, StatusSeverity::none},
			{Status::aborted_by_host, StatusSeverity::warning},
			{Status::interrupted, StatusSeverity::warning},
			{Status::fatal_or_unknown, StatusSeverity::error},
			{Status::compl_unknown_failure, StatusSeverity::error},
			{Status::compl_electrical_failure, StatusSeverity::error},
			{Status::compl_servo_failure, StatusSeverity::error},
			{Status::compl_read_failure, StatusSeverity::error},
			{Status::compl_handling_damage, StatusSeverity::error},
			{Status::in_progress, StatusSeverity::none},
			{Status::reserved, StatusSeverity::none},
	};
	if (auto iter = m.find(s); iter != m.end()) {
		return iter->second;
	}
	return StatusSeverity::none;
}



std::string AtaStorageSelftestEntry::get_status_str() const
{
	return (status == Status::unknown ? status_str : get_status_displayable_name(status));
}



std::string AtaStorageSelftestEntry::format_lifetime_hours() const
{
	std::stringstream ss;
	try {
		ss.imbue(std::locale(""));
	}
	catch (const std::runtime_error& e) {
		// something is wrong with system locale, can't do anything here.
	}
	ss << std::fixed << lifetime_hours;
	return ss.str();
}



std::ostream& operator<< (std::ostream& os, const AtaStorageSelftestEntry& b)
{
	os << "Test entry " << b.test_num << ": "
		<< b.type << ", status: " << b.get_status_str() << ", remaining: " << int(b.remaining_percent);
	return os;
}



std::string AtaStorageProperty::get_section_name(AtaStorageProperty::Section s)
{
	static const std::unordered_map<Section, std::string> m {
			{Section::unknown, "unknown"},
			{Section::info, "info"},
			{Section::data, "data"},
			{Section::internal, "internal"},
	};
	if (auto iter = m.find(s); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



std::string AtaStorageProperty::get_subsection_name(AtaStorageProperty::SubSection s)
{
	static const std::unordered_map<SubSection, std::string> m {
			{SubSection::unknown, "unknown"},
			{SubSection::health, "health"},
			{SubSection::capabilities, "capabilities"},
			{SubSection::attributes, "attributes"},
			{SubSection::devstat, "devstat"},
			{SubSection::error_log, "error_log"},
			{SubSection::selftest_log, "selftest_log"},
			{SubSection::selective_selftest_log, "selective_selftest_log"},
			{SubSection::temperature_log, "temperature_log"},
			{SubSection::erc_log, "erc_log"},
			{SubSection::phy_log, "phy_log"},
			{SubSection::directory_log, "directory_log"},
	};
	if (auto iter = m.find(s); iter != m.end()) {
		return iter->second;
	}
	return "[internal_error]";
}



std::string AtaStorageProperty::get_value_type_name() const
{
	if (std::holds_alternative<std::monostate>(value))
		return "empty";
	if (std::holds_alternative<std::string>(value))
		return "string";
	if (std::holds_alternative<int64_t>(value))
		return "integer";
	if (std::holds_alternative<bool>(value))
		return "bool";
	if (std::holds_alternative<std::chrono::seconds>(value))
		return "time_length";
	if (std::holds_alternative<AtaStorageCapability>(value))
		return "capability";
	if (std::holds_alternative<AtaStorageAttribute>(value))
		return "attribute";
	if (std::holds_alternative<AtaStorageStatistic>(value))
		return "statistic";
	if (std::holds_alternative<AtaStorageErrorBlock>(value))
		return "error_block";
	if (std::holds_alternative<AtaStorageSelftestEntry>(value))
		return "selftest_entry";
	return "[internal_error]";
}



bool AtaStorageProperty::empty() const
{
	return std::holds_alternative<std::monostate>(value);
}



void AtaStorageProperty::dump(std::ostream& os, std::size_t internal_offset) const
{
	std::string offset(internal_offset, ' ');

	os << offset << "[" << get_section_name(section)
			<< (section == Section::data ? (", " + get_subsection_name(subsection)) : "") << "]"
			<< " " << generic_name
			// << (generic_name == reported_name ? "" : (" (" + reported_name + ")"))
			<< ": [" << get_value_type_name() << "] ";

	// if (!readable_value.empty())
	// 	os << readable_value;

	if (std::holds_alternative<std::monostate>(value)) {
		os << "[empty]";
	} else if (std::holds_alternative<std::string>(value)) {
		os << std::get<std::string>(value);
	} else if (std::holds_alternative<int64_t>(value)) {
		os << std::get<int64_t>(value) << " [" << reported_value << "]";
	} else if (std::holds_alternative<bool>(value)) {
		os << std::string(std::get<bool>(value) ? "Yes" : "No") << " [" << reported_value << "]";
	} else if (std::holds_alternative<std::chrono::seconds>(value)) {
		os << std::get<std::chrono::seconds>(value).count() << " sec [" << reported_value << "]";
	} else if (std::holds_alternative<AtaStorageCapability>(value)) {
		os << std::get<AtaStorageCapability>(value);
	} else if (std::holds_alternative<AtaStorageAttribute>(value)) {
		os << std::get<AtaStorageAttribute>(value);
	} else if (std::holds_alternative<AtaStorageStatistic>(value)) {
		os << std::get<AtaStorageStatistic>(value);
	} else if (std::holds_alternative<AtaStorageErrorBlock>(value)) {
		os << std::get<AtaStorageErrorBlock>(value);
	} else if (std::holds_alternative<AtaStorageSelftestEntry>(value)) {
		os << std::get<AtaStorageSelftestEntry>(value);
	}
}



std::string AtaStorageProperty::format_value(bool add_reported_too) const
{
	if (!readable_value.empty())
		return readable_value;

	if (std::holds_alternative<std::monostate>(value))
		return "[unknown]";
	if (std::holds_alternative<std::string>(value))
		return std::get<std::string>(value);
	if (std::holds_alternative<int64_t>(value))
		return hz::number_to_string_locale(std::get<int64_t>(value)) + (add_reported_too ? (" [" + reported_value + "]") : "");
	if (std::holds_alternative<bool>(value))
		return std::string(std::get<bool>(value) ? "Yes" : "No") + (add_reported_too ? (" [" + reported_value + "]") : "");
	if (std::holds_alternative<std::chrono::seconds>(value))
		return hz::format_time_length(std::get<std::chrono::seconds>(value)) + (add_reported_too ? (" [" + reported_value + "]") : "");
	if (std::holds_alternative<AtaStorageCapability>(value))
		return hz::stream_cast<std::string>(std::get<AtaStorageCapability>(value));
	if (std::holds_alternative<AtaStorageAttribute>(value))
		return hz::stream_cast<std::string>(std::get<AtaStorageAttribute>(value));
	if (std::holds_alternative<AtaStorageStatistic>(value))
		return hz::stream_cast<std::string>(std::get<AtaStorageStatistic>(value));
	if (std::holds_alternative<AtaStorageErrorBlock>(value))
		return hz::stream_cast<std::string>(std::get<AtaStorageErrorBlock>(value));
	if (std::holds_alternative<AtaStorageSelftestEntry>(value))
		return hz::stream_cast<std::string>(std::get<AtaStorageSelftestEntry>(value));

	return "[internal_error]";
}



std::string AtaStorageProperty::get_description(bool clean) const
{
	if (clean)
		return this->description;
	return (this->description.empty() ? "No description available" : this->description);
}



void AtaStorageProperty::set_description(const std::string& descr)
{
	this->description = descr;
}



void AtaStorageProperty::set_name(const std::string& rep_name, const std::string& gen_name, const std::string& read_name)
{
	this->reported_name = rep_name;
	this->generic_name = (gen_name.empty() ? this->reported_name : gen_name);
	this->displayable_name = (read_name.empty() ? this->reported_name : read_name);
}



std::ostream& operator<<(std::ostream& os, const AtaStorageProperty& p)
{
	p.dump(os);
	return os;
}







/// @}
