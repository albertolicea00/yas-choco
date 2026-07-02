#include <QTest>

#include "chocoadapter.h"

class TestChocoAdapter : public QObject {
    Q_OBJECT
private slots:
    void searchParsesPipeOutput()
    {
        ChocoAdapter adapter;
        const auto packages = adapter.parseSearch(QStringLiteral(
            "7zip|25.0.0\n"
            "7zip.install|25.0.0\n"));
        QCOMPARE(packages.size(), 2);
        QCOMPARE(packages.at(0).id, QStringLiteral("7zip"));
        QCOMPARE(packages.at(0).version, QStringLiteral("25.0.0"));
    }

    void outdatedParsesPinnedFlag()
    {
        ChocoAdapter adapter;
        const auto packages = adapter.parseOutdated(QStringLiteral(
            "7zip|24.0.0|25.0.0|false\n"
            "git|2.44.0|2.45.0|true\n"));
        QCOMPARE(packages.size(), 2);
        QCOMPARE(packages.at(0).installedVersion, QStringLiteral("24.0.0"));
        QCOMPARE(packages.at(0).version, QStringLiteral("25.0.0"));
        QVERIFY(!packages.at(0).pinned);
        QVERIFY(packages.at(1).pinned);
        QVERIFY(packages.at(0).outdated());
    }

    void mutatingCommandsUseYesFlag()
    {
        ChocoAdapter adapter;
        QVERIFY(adapter.installCommand("7zip", "").arguments.contains(QStringLiteral("-y")));
        QVERIFY(adapter.uninstallCommand("7zip", "").arguments.contains(QStringLiteral("-y")));
        QCOMPARE(adapter.upgradeAllCommand().arguments,
                 QStringList({"upgrade", "all", "-y"}));
    }
};

QTEST_MAIN(TestChocoAdapter)
#include "tst_chocoadapter.moc"
