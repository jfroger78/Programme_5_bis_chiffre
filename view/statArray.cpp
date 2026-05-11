// Custom includes
#include "statArray.h"
#include "ui_statArray.h"
#include "model/constants.h"

// Qt includes
#include <QStandardItemModel>
#include <QDebug>

namespace view
{
    //-------------------------------------------
    StatArray::StatArray(const SFilters p_filters,
                         QWidget* p_parent)
        : QWidget { p_parent }
        , m_ui { new Ui::StatArray }
        , m_filters { p_filters }
    //-------------------------------------------
    {
        m_ui->setupUi(this);
        m_ui->m_statGroupBox->setTitle(m_filters.filter1Str());
        m_ui->m_changeStatBtn->setVisible(!(m_filters.filter1Str().isEmpty() || m_filters.filter2Str().isEmpty()));
        m_ui->m_changeStatBtn2->setVisible(!(m_filters.filter3Str().isEmpty() || m_filters.filter4Str().isEmpty()));
        m_ui->m_activeSecondFilter->setVisible(!(m_filters.filter3Str().isEmpty() || m_filters.filter4Str().isEmpty()));
        m_ui->m_changeStatBtn->setText(m_filters.filter2Str());
        m_ui->m_changeStatBtn2->setText(m_filters.filter4Str());
        m_ui->m_statDatas->setMinimumHeight(275);

        connection();
    }

    //-------------------------------------------
    void StatArray::connection()
    //-------------------------------------------
    {
        connect(m_ui->m_start, &QPushButton::clicked, this, [this](){
                QItemSelectionModel* selectionModel = m_ui->m_statDatas->selectionModel();
                if(nullptr != selectionModel)
                {
                    emit startSubFilter(selectionModel->selectedIndexes());
                }
                else
                {
                    qWarning() << "Selection model is null";
                }
            });
        connect(m_ui->m_reset, &QPushButton::clicked, this, &StatArray::resetSubFilter);
        connect(m_ui->m_changeStatBtn, &QPushButton::clicked, this, &StatArray::changeStatFilter);
        connect(m_ui->m_changeStatBtn2, &QPushButton::clicked, this, &StatArray::changeStatFilter2);
        connect(m_ui->m_startCompo, &QPushButton::clicked, this, [this]() {
            QItemSelectionModel* selectionModel = m_ui->m_statDatas->selectionModel();
            if(nullptr != selectionModel) {
                emit startComposition(selectionModel->selectedIndexes());
            } else {
                qWarning() << "Selection model is null";
            }

        });
    }

    //-------------------------------------------
    StatArray::~StatArray()
    //-------------------------------------------
    {

    }

    //-------------------------------------------
    void StatArray::displayDatas(const StatisticsData& p_datasToDisplay)
    //-------------------------------------------
    {
        QItemSelectionModel* oldItemSelection = m_ui->m_statDatas->selectionModel();
        QList<QPair<int, int>> selectedCells;
        if(nullptr != oldItemSelection)
        {
            for(const QModelIndex& index: oldItemSelection->selectedIndexes())
            {
                selectedCells.append(QPair<int, int>(index.row(), index.column()));
            }
        }
        m_ui->m_total->setText(QString::number(p_datasToDisplay.total));

        const QStringList columnHeaders = {"P", "3", " ", "4", " ", "N", "T", "T",
                                     "P", "3", " ", "4", " ", "N", "T", "T",
                                     "P", "3", " ", "4", " ", "N", "T", "T"};

        QStandardItemModel* statModel = new QStandardItemModel(m_ui->m_statDatas);
        statModel->setHorizontalHeaderLabels(columnHeaders);

        QStringList rowHeaders;
        // Sets percents
        int rowIndex = 0;
        if(-1 != p_datasToDisplay.total) {
            for(std::map<int, std::array<int, 24>>::const_iterator it = p_datasToDisplay.datas.begin();
                it != p_datasToDisplay.datas.end();
                ++it) {
                const std::array<int, 24> totals = p_datasToDisplay.totalRowColumn.find(it->first)->second;
                rowHeaders.push_back(QString::number(it->first));
                setPercent(rowIndex,
                        it->second,
                        totals,
                        statModel);
                rowIndex++;
            }
        } else if(-1 != p_datasToDisplay.totalByColumn[0]) {
            const std::map<int, int> rowHeader = p_datasToDisplay.datasByColumn[0];
            for(std::pair<int, int> pair: rowHeader) {
                rowHeaders.push_back(QString::number(pair.first));
            }
            for(int colIndex = 0; colIndex < p_datasToDisplay.datasByColumn.size(); ++colIndex) {
                setPercent(colIndex,
                        p_datasToDisplay.datasByColumn[colIndex],
                        p_datasToDisplay.totalByColumn,
                        statModel);
            }
        } else {
            const std::map<int, int> rowHeader = p_datasToDisplay.datasByColumn[0];
            for(std::pair<int, int> pair: rowHeader) {
                rowHeaders.push_back(QString::number(pair.first));
            }

            for(int indexCol = 0; indexCol < p_datasToDisplay.datasByColumn.size(); ++indexCol) {
                std::map<int, int> colValues = p_datasToDisplay.datasByColumn[indexCol];
                int rowIndex = 0;
                for(std::pair<int, int> rowValues: colValues) {
                    QStandardItem* percentValue = nullptr;
                    if(p_datasToDisplay.totalByColumnByValue.end() != p_datasToDisplay.totalByColumnByValue.find(rowValues.first)) {
                        const float total = p_datasToDisplay.totalByColumnByValue.find(rowValues.first)->second[indexCol];
                        if(0 == total) {
                            percentValue = new QStandardItem("0");
                        } else {
                            const float value = rowValues.second;
                            percentValue = new QStandardItem(QString::number((value / total) * 100, 'f', 2));
                        }
                    } else {
                        percentValue = new QStandardItem("0");
                    }
                    statModel->setItem(rowIndex, indexCol, percentValue);
                    ++rowIndex;
                }
            }
        }

        statModel->setVerticalHeaderLabels(rowHeaders);
        getLittleOne(*statModel);
        m_ui->m_statDatas->setModel(statModel);
        m_ui->m_statDatas->resizeColumnsToContents();

        QItemSelectionModel* newItemSelection = m_ui->m_statDatas->selectionModel();
        for(const QPair<int, int>& cell: selectedCells)
        {
            const int row = cell.first;
            const int column = cell.second;
            if((row < statModel->rowCount()) && (column < statModel->columnCount()))
            {
                QModelIndex index = statModel->index(row, column);
                newItemSelection->select(index, QItemSelectionModel::Select);
            }
        }
    }

    //-------------------------------------------
    void StatArray::displayComposition(const Composition& p_compositionToDisplay)
    //-------------------------------------------
    {
        // Keep datas
        const QString keepToDisplay = "Chiffre: " + QString::number(p_compositionToDisplay.number) + ", colonne: " + p_compositionToDisplay.column;
        m_ui->m_compositionKeepInfo->setText(keepToDisplay);

        QString compositionNumbers;
        for(const std::pair<QString, int> datas: p_compositionToDisplay.keeped.composition) {
            compositionNumbers += datas.first + ": " + QString::number(datas.second) + ", ";
        }
        m_ui->m_compositionNumber->setText(compositionNumbers);
        
        QString compositionDetails;
        for(const std::pair<int, int> detailsComposition: p_compositionToDisplay.keeped.detailedComposition) {
            const int number = detailsComposition.first;
            const float numberTotal = detailsComposition.second;
            const float totalRace = p_compositionToDisplay.keeped.total;
            float percent = 0;
            if((0 != totalRace) && (0 != numberTotal)) {
                percent = (numberTotal / totalRace) * 100;
            }
            float totalTimeThisNumber = 0;
            float percent2 = 0;
            std::map<int, int>::const_iterator detailedCompositionByNumber = p_compositionToDisplay.keeped.detailByNumber.find(number);
            if(p_compositionToDisplay.keeped.detailByNumber.end() != detailedCompositionByNumber) {
                totalTimeThisNumber = detailedCompositionByNumber->second;
                percent2 = (numberTotal / totalTimeThisNumber) * 100;
            }
            compositionDetails += QString::number(number) + ": " +
                                  QString::number(numberTotal, 'f', 0) + " / " + 
                                  QString::number(totalRace, 'f', 0) + " = " +
                                  QString::number(percent, 'f', 2) + "% (" +
                                  QString::number(numberTotal) + "/" + 
                                  QString::number(totalTimeThisNumber) + " = " +
                                  QString::number(percent2, 'f', 2) + "%), ";
        }
        
        compositionDetails += "!!!";
        m_ui->m_compositionDetails->setText(compositionDetails);

        // Removed datas
        const QString removeToDisplay = "Chiffre: " + QString::number(p_compositionToDisplay.number) + ", colonne: " + p_compositionToDisplay.column;
        m_ui->m_compositionNoKeepInfos->setText(removeToDisplay);

        QString compositionNoNumbers;
        for(const std::pair<QString, int> datas: p_compositionToDisplay.removed.composition) {
            compositionNoNumbers += datas.first + ": " + QString::number(datas.second) + ", ";
        }
        m_ui->m_compositionNoNumbers->setText(compositionNoNumbers);

        QString compositionNoDetails;
        for(const std::pair<int, int> detailsComposition: p_compositionToDisplay.removed.detailedComposition) {
            compositionNoDetails += QString::number(detailsComposition.first) + ": " + QString::number(detailsComposition.second) + ", ";
        }
        compositionNoDetails += "!!!";
        m_ui->m_compositionNoDetails->setText(compositionNoDetails);

        QString numberDetailStr = "";
        
        for(const std::pair<int, int>& detailsComposition: p_compositionToDisplay.removed.numberDetail) {
            const int number = detailsComposition.first;
            const float numberTotal = detailsComposition.second;
            const float totalRace = p_compositionToDisplay.removed.total;
            float percent = 0;
            if((0 != totalRace) && (0 != numberTotal)) {
                percent = (numberTotal / totalRace) * 100;
            }
            numberDetailStr += QString::number(number) + ": " + 
                               QString::number(numberTotal, 'f', 0) + " / " +
                               QString::number(totalRace, 'f', 0) + " = " +
                               QString::number(percent, 'f', 2) + "%, ";
        }
        m_ui->m_numberDetailRemoved->setText(numberDetailStr);

        QString removedCompositionDetails;
        QString comp1;
        QString comp2;
        QString comp3;
        QString comp4;
        for(const std::pair<int, int> detailsComposition: p_compositionToDisplay.removed.detailedWinner) {
            const int number = detailsComposition.first;
            const float numberTotal = detailsComposition.second;
            const float totalRace = p_compositionToDisplay.removed.total;
            float percent0 = 0;
            if((0 != totalRace) && (0 != numberTotal)) {
                percent0 = (numberTotal / totalRace) * 100;
            }
            const float numberOfTimeWinner = detailsComposition.second;
            float numberAppearTotal = 0;
            float percent1 = 0;
            std::map<int, int>::const_iterator it = p_compositionToDisplay.removed.detailAppear.find(number);
            if(p_compositionToDisplay.removed.detailAppear.end() != it) {
                numberAppearTotal = it->second;
                percent1 = (numberOfTimeWinner / numberAppearTotal) * 100;
            }
            float numberRaceWinnerSameValue = 0;
            float percent2 = 0;
            std::map<int, int>::const_iterator it2 = p_compositionToDisplay.removed.detailWinnerWithSameValueFromCurrentRace.find(number);
            if(p_compositionToDisplay.removed.detailWinnerWithSameValueFromCurrentRace.end() != it2) {
                numberRaceWinnerSameValue = it2->second;
                percent2 = (numberRaceWinnerSameValue / numberTotal) * 100;
            }
            float numberRaceNumberAppearSameValue = 0;
            float percent3 = 0;
            std::map<int, int>::const_iterator it3 = p_compositionToDisplay.removed.detailWinnerWithSameValueAppearFromCurrentRace.find(number);
            if(p_compositionToDisplay.removed.detailWinnerWithSameValueAppearFromCurrentRace.end() != it3) {
                numberRaceNumberAppearSameValue = it3->second;
                percent3 = (numberRaceWinnerSameValue / numberRaceNumberAppearSameValue) * 100;
            }

            comp1 = QString::number(number) + ": " +
                    QString::number(numberTotal, 'f', 0) + " / " + 
                    QString::number(totalRace, 'f', 0) + " = " +
                    QString::number(percent0, 'f', 2) + "%";
            comp2 = " (" + QString::number(numberOfTimeWinner, 'f', 0) + " / " + 
                    QString::number(numberAppearTotal, 'f', 0) + " = " +
                    QString::number(percent1, 'f', 2) + "%, ";
            comp3 = QString::number(numberRaceWinnerSameValue) + "/" + 
                    QString::number(numberTotal) + " = " +
                    QString::number(percent2, 'f', 2) + "%, ";
            comp4 = QString::number(numberRaceWinnerSameValue) + "/" +
                    QString::number(numberRaceNumberAppearSameValue) + " = " +
                    QString::number(percent3, 'f', 2) + "%), ";
            removedCompositionDetails += comp1 + comp2 + comp3 + comp4;
        }
        m_ui->m_removedApparition->setText(removedCompositionDetails);
    }

    //-------------------------------------------
    void StatArray::displayRanking(const Rankings& p_rankingDatas)
    //-------------------------------------------
    {
        QStandardItemModel* rankingModel = new QStandardItemModel(m_ui->m_removedRanking);
        QStringList rowHeaders;
        int rowIndex = 0;
        for(const Ranking& removedRanking: p_rankingDatas.datasRemoved) {
            rowHeaders.push_back(QString::number(removedRanking.verticalHeaderNumber));
            QStandardItem* percent = new QStandardItem(QString::number(removedRanking.firstPercent, 'f', 2) + "%");
            rankingModel->setItem(rowIndex, 0, percent);
            QStandardItem* rank = new QStandardItem(QString::number(removedRanking.firstPercentRanking));
            rankingModel->setItem(rowIndex++, 1, rank);
        }
        rankingModel->setVerticalHeaderLabels(rowHeaders);
        m_ui->m_removedRanking->setModel(rankingModel);
        

    }

    //-------------------------------------------
    void StatArray::changeFilterText()
    //-------------------------------------------
    {
        if(0 == m_filters.filter1Str().compare(m_ui->m_changeStatBtn->text(), Qt::CaseInsensitive))
        {
            m_ui->m_changeStatBtn->setText(m_filters.filter2Str());
            m_ui->m_statGroupBox->setTitle(m_filters.filter1Str());
        }
        else
        {
            m_ui->m_changeStatBtn->setText(m_filters.filter1Str());
            m_ui->m_statGroupBox->setTitle(m_filters.filter2Str());
        }
    }

    //-------------------------------------------
    void StatArray::resetText()
    //-------------------------------------------
    {
        m_ui->m_statGroupBox->setTitle(m_filters.filter1Str());
        m_ui->m_changeStatBtn->setText(m_filters.filter2Str());
    }

    //-------------------------------------------
    void StatArray::changeFilter2Text()
    //-------------------------------------------
    {
        if(0 == m_filters.filter3Str().compare(m_ui->m_changeStatBtn2->text(), Qt::CaseInsensitive))
        {
            m_ui->m_changeStatBtn2->setText(m_filters.filter4Str());
        }
        else
        {
            m_ui->m_changeStatBtn2->setText(m_filters.filter3Str());
        }
    }

    //-------------------------------------------
    void StatArray::reset2Text()
    //-------------------------------------------
    {
        m_ui->m_changeStatBtn2->setText(m_filters.filter4Str());
    }

    //-------------------------------------------
    QAbstractItemModel* StatArray::model() const
    //-------------------------------------------
    {
        return m_ui->m_statDatas->model();
    }

    //-------------------------------------------
    const bool StatArray::isSecondFilterActivated() const
    //-------------------------------------------
    {
        return m_ui->m_activeSecondFilter->isChecked();
    }

    //-------------------------------------------
    const bool StatArray::isInverseSubFilter() const
    //-------------------------------------------
    {
        return m_ui->m_subFilterType->isChecked();
    }    

    //-------------------------------------------
    void StatArray::setPercent(const int p_rowIndex,
                               const std::array<int, 24>& p_percents,
                               const int p_total,
                               QStandardItemModel* p_model)
    //-------------------------------------------
    {
        if(0 == p_total)
        {
            for(size_t colIndex = 0; colIndex < p_percents.size(); ++colIndex)
            {
                QStandardItem* percentItem = new QStandardItem("0");
                p_model->setItem(p_rowIndex, colIndex, percentItem);
            }
            return;
        }

        for(size_t colIndex = 0; colIndex < p_percents.size(); ++colIndex)
        {
            const float percent = p_percents[colIndex];
            QStandardItem* percentItem = new QStandardItem(QString::number((percent / p_total) * 100, 'f', 2));
            p_model->setItem(p_rowIndex, colIndex, percentItem);
        }
    }

    //-------------------------------------------
    void StatArray::setPercent(const int p_rowIndex,
                               const std::array<int, 24>& p_percents,
                               const std::array<int, 24>& p_totals,
                               QStandardItemModel* p_model)
    //-------------------------------------------
    {
        for(int indexCol = 0; indexCol < 24; ++indexCol) {
            const float value = p_percents[indexCol];
            const float total = p_totals[indexCol];
            if(0 == total) {
                QStandardItem* percentValue = new QStandardItem("0");
                p_model->setItem(p_rowIndex, indexCol, percentValue);
            } else {
                QStandardItem* percentValue = new QStandardItem(QString::number((value / total) * 100, 'f', 2));
                p_model->setItem(p_rowIndex, indexCol, percentValue);
            }
        }
    }

    //-------------------------------------------
    void StatArray::setPercent(const int p_colIndex,
                               const std::map<int, int>& p_percents,
                               const std::array<int, 24>& p_totals,
                               QStandardItemModel* p_model)
    //-------------------------------------------
    {
        int rowIndex = 0;
        for(std::map<int, int>::const_iterator it = p_percents.begin();
            it != p_percents.end();
            ++it) {
            const float value = it->second;
            const float total = p_totals[p_colIndex];
            QStandardItem* percentValue = nullptr;
            if(0 == total) {
                percentValue = new QStandardItem("0");
            } else {
                percentValue = new QStandardItem(QString::number((value / total) * 100, 'f', 2));
            }
            p_model->setItem(rowIndex, p_colIndex, percentValue);
            rowIndex++;
        }
    }

    //-------------------------------------------
    void StatArray::getLittleOne(QStandardItemModel& p_model)
    //-------------------------------------------
    {
        bool isFind = false;
        for(size_t rowIndex = 0; rowIndex < p_model.rowCount(); ++rowIndex)
        {
            std::vector<int> colValue = {0};
            float littlePercent = 101.0;
            isFind = false;
            for(size_t colIndex = 0; colIndex < p_model.columnCount(); ++colIndex)
            {
                if(nullptr != p_model.item(rowIndex, colIndex))
                {
                    const float currentValue = p_model.item(rowIndex, colIndex)->text().toFloat();
                    if(littlePercent > currentValue)
                    {
                        colValue.clear();
                        littlePercent = currentValue;
                        colValue.push_back(colIndex);
                        isFind = true;
                    }
                    else if(littlePercent == currentValue)
                    {
                        colValue.push_back(colIndex);
                    }
                }
            }
            
            if(true == isFind)
            {
                for(const int col: colValue)
                {
                    p_model.item(rowIndex, col)->setBackground(QBrush(QColor(YELLOW_COLOR)));
                }
            }
        }
    }
    
}