#include "vts_panel_parse_hfp.h"

#include <ut_log.h>
#include <map>

using com::rdk::hal::panel::PQParameter;
using com::rdk::hal::panel::PanelSource;
using com::rdk::hal::videodecoder::DynamicRange;

panelHFP gPanel_hfp;

namespace
{
bool parsePQParameter(const std::string& value, PQParameter* parameter)
{
    static const std::map<std::string, PQParameter> values = {
        {"BRIGHTNESS", PQParameter::BRIGHTNESS}, {"CONTRAST", PQParameter::CONTRAST},
        {"SHARPNESS", PQParameter::SHARPNESS}, {"SATURATION", PQParameter::SATURATION},
        {"HUE", PQParameter::HUE}, {"MANUAL_BACKLIGHT", PQParameter::MANUAL_BACKLIGHT},
        {"SDR_GAMMA", PQParameter::SDR_GAMMA}, {"COLOR_TEMPERATURE", PQParameter::COLOR_TEMPERATURE},
        {"DIMMING_SETTINGS", PQParameter::DIMMING_SETTINGS},
        {"LOW_LATENCY_STATE", PQParameter::LOW_LATENCY_STATE}, {"MEMC", PQParameter::MEMC},
        {"LOCAL_CONTRAST_LEVEL", PQParameter::LOCAL_CONTRAST_LEVEL},
        {"MPEG_NOISE_REDUCTION", PQParameter::MPEG_NOISE_REDUCTION},
        {"NOISE_REDUCTION", PQParameter::NOISE_REDUCTION}, {"AI_PQ_ENGINE", PQParameter::AI_PQ_ENGINE},
        {"AMBIENT_LIGHT_SENSOR_CONTROL", PQParameter::AMBIENT_LIGHT_SENSOR_CONTROL},
        {"PRECISION_DETAIL", PQParameter::PRECISION_DETAIL}, {"DV_CALIBRATION", PQParameter::DV_CALIBRATION},
        {"TWO_POINT_WB", PQParameter::TWO_POINT_WB}, {"MULTI_POINT_WB", PQParameter::MULTI_POINT_WB},
        {"DYNAMIC_CONTRAST_LEVEL", PQParameter::DYNAMIC_CONTRAST_LEVEL}, {"AISR", PQParameter::AISR},
        {"COLOUR_MANAGEMENT_SETTINGS", PQParameter::COLOUR_MANAGEMENT_SETTINGS},
        {"TONE_MAPPING_SETTINGS", PQParameter::TONE_MAPPING_SETTINGS}};
    const auto entry = values.find(value);
    if (entry == values.end())
    {
        return false;
    }
    *parameter = entry->second;
    return true;
}

bool parseDynamicRange(const std::string& value, DynamicRange* dynamicRange)
{
    static const std::map<std::string, DynamicRange> values = {
        {"SDR", DynamicRange::SDR}, {"HLG", DynamicRange::HLG}, {"HDR10", DynamicRange::HDR10},
        {"DOLBY_VISION", DynamicRange::DOLBY_VISION}};
    const auto entry = values.find(value);
    if (entry == values.end())
    {
        return false;
    }
    *dynamicRange = entry->second;
    return true;
}

UT_status_t getStringList(const std::string& key, std::vector<std::string>* values)
{
    char value[UT_KVP_MAX_ELEMENT_SIZE] = {};
    values->clear();
    const int count = UT_KVP_PROFILE_GET_LIST_COUNT(key.c_str());
    for (int index = 0; index < count; ++index)
    {
        const std::string elementKey = key + "." + std::to_string(index);
        UT_KVP_PROFILE_GET_STRING(elementKey.c_str(), value);
        values->emplace_back(value);
    }
    return UT_STATUS_OK;
}
}

UT_status_t vts_panel_parse_hfp()
{
    const std::string baseKey = "panel";
    char value[UT_KVP_MAX_ELEMENT_SIZE] = {};

    UT_KVP_PROFILE_GET_STRING((baseKey + ".panelType").c_str(), value);
    gPanel_hfp.panelType = value;
    gPanel_hfp.pixelWidth = UT_KVP_PROFILE_GET_UINT32((baseKey + ".pixelWidth").c_str());
    gPanel_hfp.pixelHeight = UT_KVP_PROFILE_GET_UINT32((baseKey + ".pixelHeight").c_str());
    gPanel_hfp.widthCm = UT_KVP_PROFILE_GET_UINT32((baseKey + ".widthCm").c_str());
    gPanel_hfp.heightCm = UT_KVP_PROFILE_GET_UINT32((baseKey + ".heightCm").c_str());
    gPanel_hfp.frameRateMatchingSupported =
        UT_KVP_PROFILE_GET_BOOL((baseKey + ".frameRateMatchingSupported").c_str());

    const std::string ratesKey = baseKey + ".supportedRefreshRatesHz";
    gPanel_hfp.supportedRefreshRatesHz.clear();
    const int count = UT_KVP_PROFILE_GET_LIST_COUNT(ratesKey.c_str());
    for (int index = 0; index < count; ++index)
    {
        const std::string key = ratesKey + "." + std::to_string(index);
        gPanel_hfp.supportedRefreshRatesHz.push_back(
            static_cast<double>(UT_KVP_PROFILE_GET_UINT32(key.c_str())));
    }

    std::vector<std::string> strings;
    getStringList(baseKey + ".supportedPQParameters", &strings);
    gPanel_hfp.supportedPQParameters.clear();
    for (const auto& entry : strings)
    {
        PQParameter parameter;
        if (!parsePQParameter(entry, &parameter))
        {
            UT_LOG_ERROR("Invalid Panel HFP PQ parameter: %s", entry.c_str());
            return UT_STATUS_FAILURE;
        }
        gPanel_hfp.supportedPQParameters.push_back(parameter);
    }

    getStringList(baseKey + ".supportedDynamicRanges", &strings);
    gPanel_hfp.supportedDynamicRanges.clear();
    for (const auto& entry : strings)
    {
        DynamicRange dynamicRange;
        if (!parseDynamicRange(entry, &dynamicRange))
        {
            UT_LOG_ERROR("Invalid Panel HFP dynamic range: %s", entry.c_str());
            return UT_STATUS_FAILURE;
        }
        gPanel_hfp.supportedDynamicRanges.push_back(dynamicRange);
    }

    getStringList(baseKey + ".supportedColourTemperatures", &gPanel_hfp.supportedColourTemperatures);
    const std::string sourcesKey = baseKey + ".supportedAVSources";
    gPanel_hfp.supportedAVSources.clear();
    const int sourceCount = UT_KVP_PROFILE_GET_LIST_COUNT(sourcesKey.c_str());
    for (int index = 0; index < sourceCount; ++index)
    {
        PanelSource source;
        const std::string sourceKey = sourcesKey + "." + std::to_string(index);
        UT_KVP_PROFILE_GET_STRING((sourceKey + ".type").c_str(), value);
        source.type = value;
        gPanel_hfp.supportedAVSources.push_back(source);
    }
    return UT_STATUS_OK;
}
