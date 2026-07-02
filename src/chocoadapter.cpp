#include "chocoadapter.h"

using yas::CliAction;
using yas::CliCommand;
using yas::Package;

// Chocolatey adapter. Uses --limit-output (pipe-delimited) wherever choco
// supports it. NOTE: install/uninstall/upgrade/pin require Administrator —
// until the UAC helper lands (TODO etapa 4.10) the GUI itself must be run
// elevated for mutating operations.
namespace {

const QString kChoco = QStringLiteral("choco");
const QString kLimit = QStringLiteral("--limit-output");

// "name|version" or "name|current|available|pinned" lines.
QList<QStringList> pipeRows(const QString &stdOut)
{
    QList<QStringList> rows;
    const QStringList lines = stdOut.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.isEmpty() || !line.contains(QLatin1Char('|')))
            continue;
        rows.append(line.split(QLatin1Char('|')));
    }
    return rows;
}

} // namespace

QString ChocoAdapter::displayName() const { return QStringLiteral("Chocolatey"); }
QString ChocoAdapter::cliProgram() const { return kChoco; }
QStringList ChocoAdapter::cliSearchPaths() const
{
    return {QStringLiteral("C:/ProgramData/chocolatey/bin")};
}
QStringList ChocoAdapter::cliVersionArguments() const { return {QStringLiteral("--version")}; }

CliCommand ChocoAdapter::searchCommand(const QString &query) const
{
    return {kChoco, {QStringLiteral("search"), query, kLimit}};
}

CliCommand ChocoAdapter::infoCommand(const QString &packageId, const QString &) const
{
    // Full (non -r) output: the only way to get description/homepage from CLI.
    return {kChoco, {QStringLiteral("info"), packageId}};
}

CliCommand ChocoAdapter::listInstalledCommand() const
{
    return {kChoco, {QStringLiteral("list"), kLimit}};
}

CliCommand ChocoAdapter::listOutdatedCommand() const
{
    return {kChoco, {QStringLiteral("outdated"), kLimit}};
}

CliCommand ChocoAdapter::installCommand(const QString &packageId, const QString &) const
{
    return {kChoco, {QStringLiteral("install"), packageId, QStringLiteral("-y")}};
}

CliCommand ChocoAdapter::uninstallCommand(const QString &packageId, const QString &) const
{
    return {kChoco, {QStringLiteral("uninstall"), packageId, QStringLiteral("-y")}};
}

CliCommand ChocoAdapter::upgradeCommand(const QString &packageId, const QString &) const
{
    return {kChoco, {QStringLiteral("upgrade"), packageId, QStringLiteral("-y")}};
}

CliCommand ChocoAdapter::upgradeAllCommand() const
{
    return {kChoco, {QStringLiteral("upgrade"), QStringLiteral("all"), QStringLiteral("-y")}};
}

CliCommand ChocoAdapter::pinCommand(const QString &packageId, const QString &) const
{
    return {kChoco, {QStringLiteral("pin"), QStringLiteral("add"), QStringLiteral("-n"),
                     packageId}};
}

CliCommand ChocoAdapter::unpinCommand(const QString &packageId, const QString &) const
{
    return {kChoco, {QStringLiteral("pin"), QStringLiteral("remove"), QStringLiteral("-n"),
                     packageId}};
}

QList<Package> ChocoAdapter::parseSearch(const QString &stdOut) const
{
    QList<Package> result;
    const auto rows = pipeRows(stdOut);
    for (const QStringList &row : rows) {
        Package p;
        p.id = row.value(0);
        p.name = row.value(0);
        p.version = row.value(1);
        result.append(p);
    }
    return result;
}

QList<Package> ChocoAdapter::parseInfo(const QString &stdOut) const
{
    // Verbose `choco info` output: "package|version" style header line is not
    // present; fields come as " Title: X | Published: ..." and " Summary: ...".
    Package p;
    const QStringList lines = stdOut.split(QLatin1Char('\n'));
    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.startsWith(QStringLiteral("Title:"))) {
            const QString value = line.mid(6).trimmed();
            p.name = value.section(QLatin1Char('|'), 0, 0).trimmed();
        } else if (line.startsWith(QStringLiteral("Summary:"))) {
            p.description = line.mid(8).trimmed();
        } else if (line.startsWith(QStringLiteral("Software Site:"))) {
            p.homepage = line.mid(14).trimmed();
        } else if (p.id.isEmpty() && line.contains(QLatin1Char(' '))
                   && !line.contains(QLatin1Char(':'))) {
            // First "name version [tags]" line after the header.
            const QStringList tokens = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (tokens.size() >= 2 && tokens.at(1).at(0).isDigit()) {
                p.id = tokens.at(0);
                p.version = tokens.at(1);
            }
        }
    }
    if (p.id.isEmpty() && p.name.isEmpty())
        return {};
    if (p.id.isEmpty())
        p.id = p.name;
    return {p};
}

QList<Package> ChocoAdapter::parseInstalled(const QString &stdOut) const
{
    QList<Package> result;
    const auto rows = pipeRows(stdOut);
    for (const QStringList &row : rows) {
        Package p;
        p.id = row.value(0);
        p.name = row.value(0);
        p.installedVersion = row.value(1);
        p.version = row.value(1);
        result.append(p);
    }
    return result;
}

QList<Package> ChocoAdapter::parseOutdated(const QString &stdOut) const
{
    // name|installed|available|pinned
    QList<Package> result;
    const auto rows = pipeRows(stdOut);
    for (const QStringList &row : rows) {
        if (row.size() < 3)
            continue;
        Package p;
        p.id = row.value(0);
        p.name = row.value(0);
        p.installedVersion = row.value(1);
        p.version = row.value(2);
        p.pinned = row.value(3).compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0;
        result.append(p);
    }
    return result;
}

QList<CliAction> ChocoAdapter::actionCatalog() const
{
    return {
        {QStringLiteral("outdated"), tr("Show outdated"),
         tr("List packages with newer versions available"),
         {kChoco, {QStringLiteral("outdated")}}, false, false, false},
        {QStringLiteral("pins"), tr("List pins"),
         tr("Show all pinned packages"),
         {kChoco, {QStringLiteral("pin"), QStringLiteral("list"), kLimit}}, false, false, false},
        {QStringLiteral("sources"), tr("List sources"),
         tr("Show the configured package sources"),
         {kChoco, {QStringLiteral("source"), QStringLiteral("list"), kLimit}},
         false, false, false},
        {QStringLiteral("cache-clean"), tr("Clean cache"),
         tr("Remove cached downloads (requires admin)"),
         {kChoco, {QStringLiteral("cache"), QStringLiteral("remove")}}, false, true, false},
        {QStringLiteral("features"), tr("List features"),
         tr("Show chocolatey feature flags"),
         {kChoco, {QStringLiteral("feature"), QStringLiteral("list")}}, false, false, false},
        {QStringLiteral("config"), tr("Show configuration"),
         tr("Print chocolatey configuration values"),
         {kChoco, {QStringLiteral("config"), QStringLiteral("list")}}, false, false, false},
    };
}
