#ifndef STATISTIC_H
#define STATISTIC_H

// Standard c++ includes
#include <array>
#include <map>

// Qt includes
#include <QString>

struct StatisticsData
{
    // Value
    std::map<int, std::array<int, 24>> datas;
    std::array<std::map<int, int>, 24> datasByColumn;
    
    // Totals
    int total = -1;
    std::map<int, std::array<int, 24>> totalRowColumn;
    std::array<int, 24> totalByColumn = {-1};
    std::map<int, std::array<int, 24>> totalByColumnByValue = {};
};

struct Compo {
    std::map<QString, int> composition = {{"P", 0}, {"3", 0}, {"4", 0}, {"N", 0}};
    std::map<int, int> detailedComposition = {};
    std::map<int, int> detailedWinner = {};
    std::map<int, int> detailAppear = {};
    std::map<int, int> detailByNumber = {};
    std::map<int, int> detailWinnerWithSameValueFromCurrentRace = {};
    std::map<int, int> detailWinnerWithSameValueAppearFromCurrentRace = {};
    std::map<int, int> numberDetail = {};
    int total = 0;
};

struct Composition {
    int number = -1;
    QString column = "";

    Compo keeped;
    Compo removed;

    void incrementKeepType(const QString& p_type) {
        incrementType(p_type, keeped.composition);
    }

    void incrementRemovedType(const QString& p_type) {
        incrementType(p_type, removed.composition);
    }

    void incrementType(const QString& p_type, std::map<QString, int>& p_mapToIncrement) {
        std::map<QString, int>::iterator it = p_mapToIncrement.find(p_type);
        if(p_mapToIncrement.end() == it) {
            p_mapToIncrement.emplace(std::pair<QString, int>(p_type, 1));
        } else {
            it->second++;
        }
    }
};

#endif // STATISTIC_H