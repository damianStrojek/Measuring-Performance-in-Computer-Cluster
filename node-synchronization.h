//
//      node-synchronization.h - header file with functions related to synchronizing metrics gathered from different nodes
//
//      2022-2023       Damian Strojek @damianStrojek
//                      Piotr Garbowski @dideek
//                      Jakub Wasniewski @wisnia01
//

#ifndef NODE_SYNCHRONIZATION_H
#define NODE_SYNCHRONIZATION_H

// Generating MPI types
MPI_Datatype createMpiSystemMetricsType();
MPI_Datatype createMpiProcessorMetricsType();
MPI_Datatype createMpiInputOutputMetricsType();
MPI_Datatype createMpiMemoryMetricsType();
MPI_Datatype createMpiNetworkMetricsType();
MPI_Datatype createMpiPowerMetricsType();

#endif