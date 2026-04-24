#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // Initialise l'application Qt (gestion des arguments et du contexte GUI).
    QApplication a(argc, argv);
    // Crée la fenêtre principale de l'application.
    MainWindow w;
    // Affiche la fenêtre à l'écran.
    w.show();
    // Lance la boucle d'événements Qt et attend la fermeture de l'application.
    return QCoreApplication::exec();
}
