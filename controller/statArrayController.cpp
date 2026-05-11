// Custom includes
#include "statArrayController.h"
#include "tools.h"

namespace controller
{
    //--------------------------------------------------------------------------------
    StatArrayController::StatArrayController(const EFilter p_filter1,
                                             const EFilter p_filter2,
                                             const EFilter p_filter3,
                                             const EFilter p_filter4)
        : m_fullDatas {}
        , m_currentFilterType { p_filter1 }
        , m_filter1 { p_filter1 }
        , m_filter2 { p_filter2 }
        , m_currentFilter2Type { p_filter3 }
        , m_filter3 { p_filter3 }
        , m_filter4 { p_filter4 }
        , m_statArray { {p_filter1,
                        p_filter2,
                        p_filter3,
                        p_filter4} }
        , m_currentRaceDatas { -1 }
        , m_isChangeFilterConnected { false }
    //--------------------------------------------------------------------------------
    {
        connect(&m_statArray, &view::StatArray::startSubFilter, this, &StatArrayController::onStartSubFilter);
        connect(&m_statArray, &view::StatArray::resetSubFilter, this, &StatArrayController::onResetSubFilter);
        connect(&m_statArray, &view::StatArray::startComposition, this, &StatArrayController::onStartComposition);
    }

    //--------------------------------------------------------------------------------
    StatArrayController::~StatArrayController()
    //--------------------------------------------------------------------------------
    {

    }

    //--------------------------------------------------------------------------------
    void StatArrayController::setFullDatas(const std::vector<RaceData> p_fullDatas)
    //--------------------------------------------------------------------------------
    {
        m_fullDatas = p_fullDatas;
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::setCurrentRaceDatas(const std::array<int, 24> p_currentRaceDatas)
    //--------------------------------------------------------------------------------
    {
        m_currentRaceDatas = p_currentRaceDatas;
    }

    //--------------------------------------------------------------------------------
    view::StatArray& StatArrayController::statArrayHMI()
    //--------------------------------------------------------------------------------
    {
        return m_statArray;
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::onStartFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        m_currentFilterType = m_filter1;
        m_currentFilter2Type = m_filter3;
        m_statArray.resetText();
        filteredDatas(p_filter, m_currentFilterType);
        StatisticsData statistics;
        switch(m_currentFilterType) {
            case FilterDiscipline:
            case FilterLeaver:
            case FilterDistance:
            case FilterYellow:
            case FilterLPJ:
            case FilterLP:
            case FilterLPD:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatas);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatas);
                }
                break;
            case FilterColorFull:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByColorsByColumn);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByColorsByColumn);
                }
                break;
            case FilterColor:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByNumbers);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByNumbers);
                }
                break;
            default:
                break;
        }
        m_statArray.displayDatas(statistics);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::filteredDatas(const Filter& p_filter,
                                            const EFilter& p_filterType)
    //--------------------------------------------------------------------------------
    {
        switch(p_filterType)
        {
            case FilterLPJ:
                m_currentFilteredDatas = startLPJFilter(p_filter);
                break;
            case FilterLP:
                m_currentFilteredDatas = startLPFilter(p_filter);
                break;
            case FilterLPD:
                m_currentFilteredDatas = startLPDFilter(p_filter);
                break;
            case FilterColorFull:
                m_currentFilteredDatasByColorsByColumn = startColorFullFilter(p_filter);
                break;
            case FilterColor:
                m_currentFilteredDatasByNumbers = startColorFilter(p_filter);
                break;
            default:
                break;
        }
    }

    //--------------------------------------------------------------------------------
    const bool StatArrayController::isFilterLP(const Filter& p_filter,
                                               const QString& p_letterToFilter,
                                               const int p_nbrLeaverToFilter)
    //--------------------------------------------------------------------------------
    {
        if(p_filter.isLetterFilter(p_letterToFilter))
        {
            if(p_filter.isNbrLeaverFilter(p_nbrLeaverToFilter))
            {
                return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------------------------
    const bool StatArrayController::isFilterLPJ(const Filter& p_filter,
                                                const QString& p_letterToFilter,
                                                const int p_nbrLeaverToFilter,
                                                const int p_yellowToFilter)
    //--------------------------------------------------------------------------------
    {
        if(isFilterLP(p_filter, p_letterToFilter, p_nbrLeaverToFilter))
        {
            if(p_filter.isYellowFilter(p_yellowToFilter))
            {
                return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------------------------
    const bool StatArrayController::isFilterLPD(const Filter& p_filter,
                                                const QString& p_letterToFilter,
                                                const int p_nbrLeaverToFilter,
                                                const int p_distanceToFilter)
    //--------------------------------------------------------------------------------
    {
        if(isFilterLP(p_filter, p_letterToFilter, p_nbrLeaverToFilter))
        {
            if(p_filter.isDistanceFilter(p_distanceToFilter))
            {
                return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------------------------
    const std::vector<RaceData> StatArrayController::startLPJFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        std::vector<RaceData> result;
        if(m_fullDatas.empty())
        {
            qWarning() << "Set the full datas before using startLPJFilter";
            return result;
        }

        for(const RaceData& data: m_fullDatas)
        {
            const int yellowCounter = computeYellow(m_currentRaceDatas, data.totalDatas());
            if(isFilterLPJ(p_filter,
                           data.discipline,
                           data.nbrLeaver,
                           yellowCounter))
            {
                result.push_back(data);
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    const std::vector<RaceData> StatArrayController::startLPFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        std::vector<RaceData> result;
        if(m_fullDatas.empty())
        {
            qWarning() << "Set the full datas before using startLPJFilter";
            return result;
        }

        for(const RaceData& data: m_fullDatas)
        {
            if(isFilterLP(p_filter,
                          data.discipline,
                          data.nbrLeaver))
            {
                result.push_back(data);
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    const std::vector<RaceData> StatArrayController::startLPDFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        std::vector<RaceData> result;
        if(m_fullDatas.empty())
        {
            qWarning() << "Set the full datas before using startLPJFilter";
            return result;
        }

        for(const RaceData& data: m_fullDatas)
        {
            if(isFilterLPD(p_filter,
                           data.discipline,
                           data.nbrLeaver,
                           data.distance))
            {
                result.push_back(data);
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    std::array<std::vector<RaceData>, 24> StatArrayController::startColorFullFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        std::array<std::vector<RaceData>, 24> result;
        if(m_fullDatas.empty()) {
            qWarning() << "Set the full datas before using startColorFullFilter";
            return result;
        }

        for(const RaceData& data: m_fullDatas) {
            const std::map<int, NumberValue> raceDataNumberOfNumbers = data.numberOfNumbers();
            for(std::map<int, NumberValue>::const_iterator it = raceDataNumberOfNumbers.begin();
                it != raceDataNumberOfNumbers.end();
                ++it) {
                if(p_filter.numberFilter.end() != p_filter.numberFilter.find(it->first)) {
                    std::map<int, NumberValue>::const_iterator value = p_filter.numberFilter.find(it->first);
                    bool isSatisfyFilter = true;
                    for(std::map<int, int>::const_iterator itFilter = value->second.numbersValue.begin();
                        itFilter != value->second.numbersValue.end();
                        ++itFilter) {
                        std::map<int, int>::const_iterator itData = it->second.numbersValue.find(itFilter->first);
                        if((itData == it->second.numbersValue.end()) || (itData->second != itFilter->second)) {
                            isSatisfyFilter = false;
                            break;
                        }
                    }
                    if(isSatisfyFilter) {
                        result[it->first].push_back(data);
                    }
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    std::map<int, std::array<std::vector<RaceData>, 24>> StatArrayController::startColorFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        // Todo to change to have RaceData by color and by column
        std::map<int, std::array<std::vector<RaceData>, 24>> result;
        if(m_fullDatas.empty()) {
            qWarning() << "Set the full datas before using startColorFilter";
            return result;
        }

        for(const RaceData& datas: m_fullDatas) {
            const std::map<int, NumberValue> raceDataNumberOfColor = datas.numberOfNumbers();
            for(std::map<int, NumberValue>::const_iterator raceColumn = raceDataNumberOfColor.begin();
                raceColumn != raceDataNumberOfColor.end();
                ++raceColumn) {
                    // it->first = colIndex
                    // it->second.first = number
                    // it->second.second = number of number value
                std::map<int, NumberValue>::const_iterator filteredColumn = p_filter.numberFilter.find(raceColumn->first);
                if(p_filter.numberFilter.end() == filteredColumn) {
                    continue;
                }
                const std::map<int, int> raceNumberValue = raceColumn->second.numbersValue;
                const std::map<int, int> filteredNumberValue = filteredColumn->second.numbersValue;
                for(std::map<int, int>::const_iterator raceValue = raceNumberValue.begin();
                    raceValue != raceNumberValue.end();
                    ++raceValue) {
                    if(filteredNumberValue.end() == filteredNumberValue.find(raceValue->first)) {
                        continue;
                    }
                    if(raceValue->second == filteredNumberValue.find(raceValue->first)->second) {
                        result[raceValue->first][raceColumn->first].push_back(datas);
                    }
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    const int StatArrayController::computeYellow(const std::array<int, 24>& p_currentRace,
                                                 const std::array<int, 24> p_dataFromDB)
    //--------------------------------------------------------------------------------
    {
        int yellowCounter = 0;
        if(-1 == p_dataFromDB[0])
        {
            return yellowCounter;
        }
        for(size_t index = 0; index < p_currentRace.size(); ++index)
        {
            if(p_currentRace[index] == p_dataFromDB[index])
            {
                yellowCounter++;
            }
        }

        return yellowCounter;
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::generateStatistics(const std::vector<RaceData>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        m_tmpFilteredDatas = p_filteredDatas;
        m_tmpFilteredDatasByColorsByColumn = {};
        m_tmpFilteredDatasByNumbers.clear();
        StatisticsData result;
        result = findRowNumbers();
        result.total = p_filteredDatas.size();
        for(const RaceData& data: p_filteredDatas) {
            for(const std::pair<int, std::array<int, 24>>& countTotal: result.totalRowColumn) {
                for(int indexCol = 0; indexCol < 24; ++indexCol) {
                    if(data.numberInColumn(countTotal.first, indexCol)) {
                        result.totalRowColumn[countTotal.first][indexCol]++;
                    }
                }
            }
        }
        for(const RaceData& data: p_filteredDatas) {
            for(int indexCol = 0; indexCol < 24; ++indexCol) {
                const int currentData = data.winnerDatas()[indexCol];
                if(-1 == currentData) {
                    break;
                }

                std::map<int, std::array<int, 24>>::iterator it = result.datas.find(currentData);
                if(it != result.datas.end()) {
                    it->second[indexCol]++;
                }
                
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::generateStatistics(const std::array<std::vector<RaceData>, 24>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        m_tmpFilteredDatas.clear();
        m_tmpFilteredDatasByColorsByColumn = p_filteredDatas;
        m_tmpFilteredDatasByNumbers.clear();
        StatisticsData result;
        result = findRowNumbers();

        for(int colIndex = 0; colIndex < p_filteredDatas.size(); ++colIndex) {
            std::map<int, int> tmpColMap;
            // Initialize map with the number find in the current race
            for(std::pair<int, std::array<int, 24>> colData: result.datas) {
                tmpColMap.emplace(std::pair<int, int>(colData.first, 0));
            }
            std::vector<RaceData> currentRaces = p_filteredDatas[colIndex];
            result.totalByColumn[colIndex] = currentRaces.size();
            for(const RaceData& data: currentRaces) {
                const int currentData = data.winnerDatas()[colIndex];
                if(-1 == currentData) {
                    break;
                }
                std::map<int, int>::iterator it = tmpColMap.find(currentData);
                if(it != tmpColMap.end()) {
                    it->second++;
                }
            }
            result.datasByColumn[colIndex] = tmpColMap;
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::generateStatistics(
        const std::map<int, std::array<std::vector<RaceData>, 24>>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        m_tmpFilteredDatas.clear();
        m_tmpFilteredDatasByColorsByColumn = {};
        m_tmpFilteredDatasByNumbers = p_filteredDatas;
        StatisticsData result;
        result = findRowNumbers();

        for(int colIndex = 0; colIndex < 24; ++colIndex) {
            std::map<int, int> tmpColMap;
            // Initialize map with the number find in the current race
            for(std::pair<int, std::array<int, 24>> colData: result.datas) {
                tmpColMap.emplace(std::pair<int, int>(colData.first, 0));
                result.totalByColumnByValue[colData.first] = {};
            }
            result.datasByColumn[colIndex] = tmpColMap;
        }

        for(std::map<int, std::array<int, 24>>::const_iterator it = result.datas.begin();
            it != result.datas.end();
            ++it) {
            std::array<std::vector<RaceData>, 24> values = {};
            if(p_filteredDatas.end() != p_filteredDatas.find(it->first)) {
                values = p_filteredDatas.find(it->first)->second;
                for(int colIndex = 0; colIndex < values.size(); ++colIndex) {
                    result.totalByColumnByValue[it->first][colIndex] = values[colIndex].size();
                    for(const RaceData& data: values[colIndex]) {
                        // it->first = number
                        if(data.isWinnerPassFilter(it->first, colIndex)) {
                            result.datasByColumn[colIndex][it->first]++;
                        }
                    }

                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::onStartSubFilter(const QModelIndexList& p_filterList)
    //--------------------------------------------------------------------------------
    {
        if(m_currentFilteredDatas.empty() &&
            m_currentFilteredDatasByColorsByColumn.empty() &&
            m_currentFilteredDatasByNumbers.empty())
        {
            qWarning() << "No current filtered datas, start generic statistics before use onStartSubFilter";
            return;
        }

        StatisticsData statistics;
        switch(m_currentFilterType) {
            case FilterDiscipline:
            case FilterLeaver:
            case FilterDistance:
            case FilterYellow:
            case FilterLPJ:
            case FilterLP:
            case FilterLPD:
            {
                std::vector<RaceData> datas = m_currentFilteredDatas;
                std::vector<RaceData> tmpData;
                for(const QModelIndex& modelIndex: p_filterList)
                {
                    const int rowIndex = modelIndex.row();
                    const int colIndex = modelIndex.column();
                    const int value = statArrayHMI().model()->headerData(rowIndex, Qt::Vertical).toInt();
                    for(const RaceData& data: datas)
                    {
                        // We want all data which do not corresponding to the filter
                        if(m_statArray.isInverseSubFilter() == data.isWinnerPassFilter(value, colIndex))
                        {
                            tmpData.push_back(data);
                        }
                    }
                    datas.clear();
                    datas = tmpData;
                    tmpData.clear();
                }

                statistics = generateStatistics(datas);
                break;
            }
            case FilterColorFull:
            {
                std::array<std::vector<RaceData>, 24> datas = m_currentFilteredDatasByColorsByColumn;
                std::vector<RaceData> tmpData = {};
                for(const QModelIndex& modelIndex: p_filterList) {
                    const int rowIndex = modelIndex.row();
                    const int colIndex = modelIndex.column();
                    const int value = statArrayHMI().model()->headerData(rowIndex, Qt::Vertical).toInt();
                    std::vector<RaceData> currentColDatas = datas[colIndex];
                    for(const RaceData& data: currentColDatas) {
                        if(m_statArray.isInverseSubFilter() == data.isWinnerPassFilter(value, colIndex)) {
                            tmpData.push_back(data);
                        }
                    }
                    datas[colIndex].clear();
                    datas[colIndex] = tmpData;
                    tmpData.clear();
                }
                statistics = generateStatistics(datas);
                break;
            }
            case FilterColor:
            {
                std::map<int, std::array<std::vector<RaceData>, 24>> datas = m_currentFilteredDatasByNumbers;
                std::vector<RaceData> tmpData = {};
                for(const QModelIndex& modelIndex: p_filterList) {
                    const int rowIndex = modelIndex.row();
                    const int colIndex = modelIndex.column();
                    const int value = statArrayHMI().model()->headerData(rowIndex, Qt::Vertical).toInt();
                    const std::map<int, std::array<std::vector<RaceData>, 24>>::iterator it = datas.find(value);
                    if(datas.end() != it) {
                        std::vector<RaceData> datasVec = it->second[colIndex];
                        for(const RaceData& data: datasVec) {
                            if(m_statArray.isInverseSubFilter() == data.isWinnerPassFilter(value, colIndex)) {
                                tmpData.push_back(data);
                            }
                        }
                        datas[value][colIndex].clear();
                        datas[value][colIndex] = tmpData;
                        tmpData.clear();
                    }
                    tmpData.clear();
                }
                statistics = generateStatistics(datas);
                break;
            }
            default:
                break;
        }
        
        m_statArray.displayDatas(statistics);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::onStartComposition(const QModelIndexList& p_filterList)
    //--------------------------------------------------------------------------------
    {
        std::vector<int> keepRaces = {};
        std::vector<int> removeRaces = {};
        Composition currentComposition;
        for(const QModelIndex& modelIndex: p_filterList)
        {
            const int rowIndex = modelIndex.row();
            const int colIndex = modelIndex.column();

            const int rowNumber = statArrayHMI().model()->headerData(rowIndex, Qt::Vertical).toInt();
            const QString colHeader = statArrayHMI().model()->headerData(colIndex, Qt::Horizontal).toString();
            
            currentComposition.number = rowNumber;
            if((!colHeader.isEmpty()) && (" " != colHeader)) {
                currentComposition.column = colHeader;
            } else if((2 == colIndex) || (10 == colIndex) || (18 == colIndex)) {
                currentComposition.column = "3";
            } else if((4 == colIndex) || (12 == colIndex) || (20 == colIndex)) {
                currentComposition.column = "4";
            }

            bool secondTmpFilteredIsEmpty = true;
            for(std::vector<RaceData> datas: m_tmpFilteredDatasByColorsByColumn) {
                if(!datas.empty()) {
                    secondTmpFilteredIsEmpty = false;
                    break;
                }
            }

            if(0 != m_tmpFilteredDatas.size()) {
                computeDetails(
                    currentComposition,
                    keepRaces,
                    removeRaces,
                    m_tmpFilteredDatas,
                    rowNumber,
                    colIndex);
            } else if(!secondTmpFilteredIsEmpty) {
                std::vector<RaceData> currentColDatas = m_tmpFilteredDatasByColorsByColumn[colIndex];
                computeDetails(
                    currentComposition,
                    keepRaces,
                    removeRaces,
                    currentColDatas,
                    rowNumber,
                    colIndex);
            } else if(0 != m_tmpFilteredDatasByNumbers.size()) {
                const std::map<int, std::array<std::vector<RaceData>, 24>>::iterator it = m_tmpFilteredDatasByNumbers.find(rowNumber);
                if(m_tmpFilteredDatasByNumbers.end() != it) {
                    const std::vector<RaceData> datas = it->second[colIndex];
                    computeDetails(
                        currentComposition,
                        keepRaces,
                        removeRaces,
                        datas,
                        rowNumber,
                        colIndex);
                }
            }
        }

        
        for(const int number: keepRaces) {

            bool isSometing = false;
            if(Tools::isP(number)) {
                currentComposition.incrementKeepType("P");
                isSometing = true;
            }

            if(Tools::is3(number)) {
                currentComposition.incrementKeepType("3");
                isSometing = true;
            }
            if(Tools::is4(number)) {
                currentComposition.incrementKeepType("4");
                isSometing = true;
            }
            if(!isSometing) {
                currentComposition.incrementKeepType("N");
            }

            currentComposition.keeped.detailedComposition[number]++;
        }
        for(const int number: removeRaces) {

            bool isSometing = false;
            if(Tools::isP(number)) {
                currentComposition.incrementRemovedType("P");
                isSometing = true;
            }

            if(Tools::is3(number)) {
                currentComposition.incrementRemovedType("3");
                isSometing = true;
            }
            if(Tools::is4(number)) {
                currentComposition.incrementRemovedType("4");
                isSometing = true;
            }
            if(!isSometing) {
                currentComposition.incrementRemovedType("N");
            }
            currentComposition.removed.detailedComposition[number]++;
        }

        m_statArray.displayComposition(currentComposition);

        Rankings rankings;
        rankingDetails(rankings, p_filterList);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::computeDetails(
        Composition& p_details,
        std::vector<int>& p_keepedRaces,
        std::vector<int>& p_removedRaces,
        const std::vector<RaceData>& p_datas,
        const int p_rowNumber,
        const int p_colIndex)
    //--------------------------------------------------------------------------------
    {
        for(const RaceData& data: p_datas) {

            if(!data.numberInColumn(p_rowNumber, p_colIndex)) {
                continue;
            }
            
            const std::vector<int> foundedNumbers = data.getNumberCorrespondingToValue(p_rowNumber, p_colIndex);
            if(!data.isWinnerPassFilter(p_rowNumber, p_colIndex)) {
                compositionRemoved(
                    p_details,
                    p_removedRaces,
                    data,
                    p_colIndex
                );
                
            } else {
                compositionKeeped(
                    p_details,
                    p_keepedRaces,
                    data,
                    foundedNumbers);
            }

            for(const int foundNumber: foundedNumbers) {
                for(const int currentHorse: m_currentRace->numbers()) {
                    if(foundNumber == currentHorse) {
                        p_details.removed.detailWinnerWithSameValueAppearFromCurrentRace[currentHorse]++;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::compositionKeeped(
        Composition& p_details,
        std::vector<int>& p_keepedRaces,
        const RaceData& p_data,
        const std::vector<int> p_foundedNumbers)
    //--------------------------------------------------------------------------------
    {
        p_details.keeped.total++;
        p_keepedRaces.push_back(p_data.winner);
        for(const int number: p_foundedNumbers) {
            p_details.keeped.detailByNumber[number]++;
        }
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::compositionRemoved(
        Composition& p_details,
        std::vector<int>& p_removedRaces,
        const RaceData& p_data,
        const int p_colIndex
    )
    //--------------------------------------------------------------------------------
    {
        p_details.removed.total++;
        p_details.removed.detailedWinner[p_data.winner]++;
        for(size_t i = 0; i < m_currentRace->numbers().size(); ++i) {
            const int currentHorse = m_currentRace->numbers()[i];
            if(currentHorse == p_data.winner) {
                int currentHorseValue = 0;
                if(8 > p_colIndex) {
                    QModelIndex index = m_currentRace->modelEn2()->index(i, p_colIndex);
                    currentHorseValue = m_currentRace->modelEn2()->data(index).toInt();
                } else if((8 <= p_colIndex) && (16 > p_colIndex)) {
                    QModelIndex index = m_currentRace->modelEn3()->index(i, p_colIndex - 8);
                    currentHorseValue = m_currentRace->modelEn3()->data(index).toInt();
                } else {
                    QModelIndex index = m_currentRace->modelEn2En3()->index(i, p_colIndex - 16);
                    currentHorseValue = m_currentRace->modelEn2En3()->data(index).toInt();
                }
                if(currentHorseValue == p_data.winnerDatas()[p_colIndex]) {
                    p_details.removed.detailWinnerWithSameValueFromCurrentRace[p_data.winner]++;
                }
            }
        }
        for(const int number: p_data.numbers) {
            p_details.removed.detailAppear[number]++;
        }
        p_removedRaces.push_back(p_data.winner);
        p_details.removed.numberDetail[p_data.winnerDatas()[p_colIndex]]++;
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::rankingDetails(Rankings& p_ranking, const QModelIndexList& p_filterList)
    //--------------------------------------------------------------------------------
    {
        bool isSelectedCell = false;
        std::vector<int> keepRaces = {};
        std::vector<int> removeRaces = {};
        Composition currentComposition;
        for(int rowIndex = 0; rowIndex < statArrayHMI().model()->rowCount(); ++rowIndex) {
            for(int colIndex = 0; colIndex < statArrayHMI().model()->columnCount(); ++colIndex) {
                for(const QModelIndex& modelIndex: p_filterList)
                {
                    const int filterRowIndex = modelIndex.row();
                    const int filterColIndex = modelIndex.column();
                    if((filterRowIndex == rowIndex) && (filterColIndex == colIndex)) {
                        isSelectedCell = true;
                        break;
                    }
                }
                if(isSelectedCell) {
                    isSelectedCell = false;
                    continue;
                }

                const int rowNumber = statArrayHMI().model()->headerData(rowIndex, Qt::Vertical).toInt();
                const QString colHeader = statArrayHMI().model()->headerData(colIndex, Qt::Horizontal).toString();
                
                currentComposition.number = rowNumber;
                if((!colHeader.isEmpty()) && (" " != colHeader)) {
                    currentComposition.column = colHeader;
                } else if((2 == colIndex) || (10 == colIndex) || (18 == colIndex)) {
                    currentComposition.column = "3";
                } else if((4 == colIndex) || (12 == colIndex) || (20 == colIndex)) {
                    currentComposition.column = "4";
                }

                bool secondTmpFilteredIsEmpty = true;
                for(std::vector<RaceData> datas: m_tmpFilteredDatasByColorsByColumn) {
                    if(!datas.empty()) {
                        secondTmpFilteredIsEmpty = false;
                        break;
                    }
                }

                if(0 != m_tmpFilteredDatas.size()) {
                    computeDetails(
                        currentComposition,
                        keepRaces,
                        removeRaces,
                        m_tmpFilteredDatas,
                        rowNumber,
                        colIndex);
                } else if(!secondTmpFilteredIsEmpty) {
                    std::vector<RaceData> currentColDatas = m_tmpFilteredDatasByColorsByColumn[colIndex];
                    computeDetails(
                        currentComposition,
                        keepRaces,
                        removeRaces,
                        currentColDatas,
                        rowNumber,
                        colIndex);
                } else if(0 != m_tmpFilteredDatasByNumbers.size()) {
                    const std::map<int, std::array<std::vector<RaceData>, 24>>::iterator it = m_tmpFilteredDatasByNumbers.find(rowNumber);
                    if(m_tmpFilteredDatasByNumbers.end() != it) {
                        const std::vector<RaceData> datas = it->second[colIndex];
                        computeDetails(
                            currentComposition,
                            keepRaces,
                            removeRaces,
                            datas,
                            rowNumber,
                            colIndex);
                    }
                }
            }
        }

        std::array<int, 8> values;
        std::array<int, 8> totals;
        std::array<float, 8> percents;
        for(int indexNumber = 0; indexNumber < m_currentRace->numbers().size(); ++indexNumber) {
            const int number = m_currentRace->numbers()[indexNumber];
            std::map<int, int>::iterator itValue = currentComposition.removed.detailedWinner.find(number);
            values[indexNumber] = 0;
            totals[indexNumber] = 0;
            percents[indexNumber] = 0.0;
            if(currentComposition.removed.detailedWinner.end() != itValue){
                values[indexNumber] = itValue->second;
                totals[indexNumber] = currentComposition.removed.total;
                percents[indexNumber] = (float(values[indexNumber]) / float(totals[indexNumber])) * 100;
            }
            p_ranking.datasRemoved[indexNumber].verticalHeaderNumber = number;
            p_ranking.datasRemoved[indexNumber].value = values[indexNumber];
            p_ranking.datasRemoved[indexNumber].total = totals[indexNumber];
            p_ranking.datasRemoved[indexNumber].firstPercent = percents[indexNumber];
        }
        
        std::array<float, 8> tmpPercents = percents;
        std::sort(tmpPercents.begin(), tmpPercents.end(), std::greater<int>());
        // Remove duplicate values
        auto last = std::unique(tmpPercents.begin(), tmpPercents.end());
        // Fill rest with 0 (if needed)
        std::fill(last, tmpPercents.end(), 0);

        for(int tmpIndex = 0; tmpIndex < tmpPercents.size(); ++tmpIndex) {
            for(int percentIndex = 0; percentIndex < percents.size(); ++percentIndex) {
                if(tmpPercents[tmpIndex] == percents[percentIndex]) {
                    p_ranking.datasRemoved[percentIndex].firstPercentRanking = tmpIndex + 1;
                }
            }
        }

        m_statArray.displayRanking(p_ranking);

    }

    //--------------------------------------------------------------------------------
    void StatArrayController::onResetSubFilter()
    //--------------------------------------------------------------------------------
    {
        StatisticsData statistics;
        switch(m_currentFilterType) {
            case FilterDiscipline:
            case FilterLeaver:
            case FilterDistance:
            case FilterYellow:
            case FilterLPJ:
            case FilterLP:
            case FilterLPD:
                statistics = generateStatistics(m_currentFilteredDatas);
                break;
            case FilterColorFull:
                statistics = generateStatistics(m_currentFilteredDatasByColorsByColumn);
                break;
            case FilterColor:
                statistics = generateStatistics(m_currentFilteredDatasByNumbers);
                break;
            default:
                break;
        }
        m_statArray.displayDatas(statistics);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::changeStatFilter(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        if(m_currentFilterType == m_filter1)
        {
            m_currentFilterType = m_filter2;
        }
        else
        {
            m_currentFilterType = m_filter1;
        }
        m_statArray.changeFilterText();
        filteredDatas(p_filter, m_currentFilterType);
        StatisticsData statistics;
        switch(m_currentFilterType) {
            case FilterDiscipline:
            case FilterLeaver:
            case FilterDistance:
            case FilterYellow:
            case FilterLPJ:
            case FilterLP:
            case FilterLPD:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatas);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatas);
                }
                break;
            case FilterColorFull:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByColorsByColumn);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByColorsByColumn);
                }
                break;
            case FilterColor:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByNumbers);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByNumbers);
                }
                break;
            default:
                break;
        }
        m_statArray.displayDatas(statistics);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::changeStatFilter2(const Filter& p_filter)
    //--------------------------------------------------------------------------------
    {
        if(m_currentFilter2Type == m_filter3)
        {
            m_currentFilter2Type = m_filter4;
        }
        else
        {
            m_currentFilter2Type = m_filter3;
        }
        m_statArray.changeFilter2Text();
        filteredDatas(p_filter, m_currentFilterType);
        StatisticsData statistics;
        switch(m_currentFilterType) {
            case FilterDiscipline:
            case FilterLeaver:
            case FilterDistance:
            case FilterYellow:
            case FilterLPJ:
            case FilterLP:
            case FilterLPD:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatas);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatas);
                }
                break;
            case FilterColorFull:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByColorsByColumn);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByColorsByColumn);
                }
                break;
            case FilterColor:
                if(EFilter::FilterNone != m_currentFilter2Type) {
                    statistics = applySecondFilter(
                        p_filter,
                        m_currentFilter2Type,
                        m_currentFilteredDatasByNumbers);
                } else {
                    statistics = generateStatistics(m_currentFilteredDatasByNumbers);
                }
                break;
            default:
                break;
        }
        m_statArray.displayDatas(statistics);
    }

    //--------------------------------------------------------------------------------
    void StatArrayController::setRace(view::Course* p_currentRace)
    //--------------------------------------------------------------------------------
    {
        m_currentRace = p_currentRace;
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::applySecondFilter(
        const Filter& p_currentFilter,
        const EFilter& p_filter,
        const std::vector<RaceData>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        std::vector<RaceData> tmpDatas = {};
        if(m_statArray.isSecondFilterActivated()) {
            for(const RaceData& data: p_filteredDatas) {
                switch(p_filter) {
                    case FilterLPJ:
                    {
                        const int yellowCounter = computeYellow(m_currentRaceDatas, data.totalDatas());
                        if(isFilterLPJ(p_currentFilter,
                                    data.discipline,
                                    data.nbrLeaver,
                                    yellowCounter))
                        {
                            tmpDatas.push_back(data);
                        }
                        break;
                    }
                    case FilterLP:
                    {
                        if(isFilterLP(p_currentFilter,
                                    data.discipline,
                                    data.nbrLeaver))
                        {
                            tmpDatas.push_back(data);
                        }
                        break;
                    }
                    default:
                        qWarning() << "Filter not yet implement" << p_filter;
                        break;
                }
            }
        } else {
            tmpDatas = p_filteredDatas;
        }

        m_currentFilteredDatas.clear();
        m_currentFilteredDatas = tmpDatas;

        return generateStatistics(tmpDatas);
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::applySecondFilter(
        const Filter& p_currentFilter,
        const EFilter& p_filter,
        const std::array<std::vector<RaceData>, 24>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        std::array<std::vector<RaceData>, 24> tmpDatas = {};
        if(m_statArray.isSecondFilterActivated()) {
            for(size_t index = 0; index < p_filteredDatas.size(); ++index) {
                const std::vector<RaceData>& dataByColumn = p_filteredDatas[index];
                std::vector<RaceData> vectorDatas = {};
                for(const RaceData& data: dataByColumn) {
                    switch(p_filter) {
                        case FilterLPJ:
                        {
                            const int yellowCounter = computeYellow(m_currentRaceDatas, data.totalDatas());
                            if(isFilterLPJ(p_currentFilter,
                                        data.discipline,
                                        data.nbrLeaver,
                                        yellowCounter))
                            {
                                vectorDatas.push_back(data);
                            }
                            break;
                        }
                        case FilterLP:
                        {
                            if(isFilterLP(p_currentFilter,
                                        data.discipline,
                                        data.nbrLeaver))
                            {
                                vectorDatas.push_back(data);
                            }
                            break;
                        }
                        default:
                            qWarning() << "Filter not yet implement" << p_filter;
                            break;
                    }
                }
                tmpDatas[index] = vectorDatas;
            }
        } else {
            tmpDatas = p_filteredDatas;
        }

        m_currentFilteredDatasByColorsByColumn = tmpDatas;

        return generateStatistics(m_currentFilteredDatasByColorsByColumn);
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::applySecondFilter(
        const Filter& p_currentFilter,
        const EFilter& p_filter,
        const std::map<int, std::array<std::vector<RaceData>, 24>>& p_filteredDatas)
    //--------------------------------------------------------------------------------
    {
        std::map<int, std::array<std::vector<RaceData>, 24>> tmpDatas = {};
        if(m_statArray.isSecondFilterActivated()) {
            for(std::map<int, std::array<std::vector<RaceData>, 24>>::const_iterator it = p_filteredDatas.begin();
                it != p_filteredDatas.end();
                ++it) {
                std::array<std::vector<RaceData>, 24> arrayDatas = {};
                for(size_t index = 0; index < it->second.size(); ++index) {
                    const std::vector<RaceData>& dataByColumn = it->second[index];
                    std::vector<RaceData> vectorDatas = {};
                    for(const RaceData& data: dataByColumn) {
                        switch(p_filter) {
                            case FilterLPJ:
                            {
                                const int yellowCounter = computeYellow(m_currentRaceDatas, data.totalDatas());
                                if(isFilterLPJ(p_currentFilter,
                                            data.discipline,
                                            data.nbrLeaver,
                                            yellowCounter))
                                {
                                    vectorDatas.push_back(data);
                                }
                                break;
                            }
                            case FilterLP:
                            {
                                if(isFilterLP(p_currentFilter,
                                            data.discipline,
                                            data.nbrLeaver))
                                {
                                    vectorDatas.push_back(data);
                                }
                                break;
                            }
                            default:
                                qWarning() << "Filter not yet implement" << p_filter;
                                break;
                        }
                    }
                    arrayDatas[index] = vectorDatas;
                }
                tmpDatas[it->first] = arrayDatas;
            }
        } else {
            tmpDatas = p_filteredDatas;
        }

        m_currentFilteredDatasByNumbers = tmpDatas;
        return generateStatistics(m_currentFilteredDatasByNumbers);
    }

    //--------------------------------------------------------------------------------
    const StatisticsData StatArrayController::findRowNumbers(const bool p_isArray)
    //--------------------------------------------------------------------------------
    {
        StatisticsData datas;

        QAbstractItemModel* en2 = m_currentRace->modelEn2();
        if(nullptr == en2) {
            qWarning() << "Error with en2 pointer";
            return datas;
        }

        QAbstractItemModel* en3 = m_currentRace->modelEn3();
        if(nullptr == en3) {
            qWarning() << "Error with en3 pointer";
            return datas;
        }

        QAbstractItemModel* en2En3 = m_currentRace->modelEn2En3();
        if(nullptr == en2En3) {
            qWarning() << "Error with en2En3 pointer";
            return datas;
        }

        for(int rowIndex = 0; rowIndex < en2->rowCount() - 1; ++rowIndex) {
            for(int colIndex: colIndexes) {
                QModelIndex en2Index = en2->index(rowIndex, colIndex);
                const int en2Value = en2->data(en2Index).toInt();
                datas.datas.emplace(std::pair<int, std::array<int, 24>>(en2Value, {}));
                datas.totalRowColumn.emplace(std::pair<int, std::array<int, 24>>(en2Value, {}));

                QModelIndex en3Index = en3->index(rowIndex, colIndex);
                const int en3Value = en3->data(en3Index).toInt();
                datas.datas.emplace(std::pair<int, std::array<int, 24>>(en3Value, {}));
                datas.totalRowColumn.emplace(std::pair<int, std::array<int, 24>>(en3Value, {}));

                QModelIndex en2En3Index = en2En3->index(rowIndex, colIndex);
                const int en2En3Value = en2En3->data(en2En3Index).toInt();
                datas.datas.emplace(std::pair<int, std::array<int, 24>>(en2En3Value, {}));
                datas.totalRowColumn.emplace(std::pair<int, std::array<int, 24>>(en2En3Value, {}));
            }
        }

        return datas;
    }
}