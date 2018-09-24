/****************************************************************************/
/// @file    FareModul.h
/// @author Ricardo Euler
/// @date    Thu, 17 August 2018
/// @version $Id$
///
// Fare Modul for calculating prices during intermodal routing
/****************************************************************************/

#include <cassert>
#include <string>
#include <vector>
#include "FareToken.h"
#include <utils/common/ToString.h> //needed by IntermodalEdge.h
#include <microsim/MSEdge.h>
#include "IntermodalEdge.h"
#include "StopEdge.h"
#include "PedestrianEdge.h"
#include "PublicTransportEdge.h"
#include "AccessEdge.h"
#include "FareZones.h"
#ifndef SUMO_FAREMODUL_H
#define SUMO_FAREMODUL_H


class ZoneCounter
{
public:
  
  explicit ZoneCounter(unsigned int ct) :
      myCount(ct) {
    
  }
  
  inline  void addZone(int zoneNumber)
  {
    zoneNumber = getOverlayZone(zoneNumber);
    if( zoneNumber == 0 )
      return;
    uint64_t repNum = fareZoneToRep[ zoneNumber ];
    //assert power of 2
    if ( bitcount(repNum) == 0 )
      return;
    myCount = myCount | repNum;
  }
  
  
  int numZones() const{
    return bitcount(myCount);
  }

  
private:
   inline int bitcount(unsigned long int intVal ) const
  {
    int count = 0;
    uint64_t counter = intVal;
    
    while( counter != 0 )
    {
      counter = counter & (counter - 1);
      ++count;
    }
    return count;
  }

private:
  uint64_t myCount;
  
  
};



/**
 * A fare state collects all the information that is necessary to compute the price. Is used as an edge label
 * in IntermodalRouter
 */
struct FareState
{
  template<class E, class L, class N, class V>
  friend class FareModul;
  
public:
  
  /** default constructor for unlabeled edges**/
  explicit  FareState():
    myFareToken( FareToken::None ),
    myCounter( std::numeric_limits<int>::max() ),
    myTravelledDistance(std::numeric_limits<double>::max() ),
    myVisistedStops(std::numeric_limits<int>::max() ),
    myPriceDiff(0)
  {
  };
  
  /**
   *
   * @param token
   */
  explicit FareState(FareToken token):
  myFareToken( token ),
  myCounter(0),
  myTravelledDistance(0),
  myVisistedStops(0),
  myPriceDiff(0){}
  
  /** Destructor **/
  ~FareState() = default;
  
  /**
   * returns true if fare state is set and not on default
   * @return if state is set
   */
      bool isValid() const {
    return !(myFareToken == FareToken::None);
  }
  
private:
  
  /** fare token **/
  FareToken myFareToken;
  /** zone counter **/
  ZoneCounter myCounter;
  /** travelled distance in km**/
  double myTravelledDistance;
  /**num of visited stops**/
  int myVisistedStops;
  /** price diff to previous edge **/
  double myPriceDiff;
  
};



struct Prices
{

  
  
  /** Prices for zones **/
  std::vector<double> zonePrices = std::vector<double>{1.9,3.4,4.9,6.2,7.7,9.2};
  double  halle = 2.3;
  double leipzig = 2.7;
  double t1 = 1.5;
  double t2 = 1.6;
  double t3 = 1.6;
  double shortTrip = 1.6;
  double shortTripLeipzig = 1.9;
  double shortTripHalle= 1.7;
  double maxPrice = 10.6;
};


/**
 * The fare modul responsible for calculating prices
 */
template<class E, class L, class N, class V>
class FareModul : public EffortCalculator<IntermodalEdge<E, L, N, V>>
{
protected:
  typedef IntermodalEdge<E, L, N, V> _IntermodalEdge;
  typedef StopEdge<E, L, N, V> _StopEdge;
  typedef PublicTransportEdge<E, L, N, V> _PublicTransportEdge;
  typedef PedestrianEdge<E, L, N, V> _PedestrianEdge;
  typedef AccessEdge<E, L, N, V> _AccessEdge;
public:
  
  /** Constructor ***/
  explicit FareModul() :
  myFareStates(),
  myEdges(nullptr)
  {};
  
   /**Implementation of EffortCalculator **/
  void init(const std::vector<_IntermodalEdge*>& edges) override
  {
    myFareStates.resize(edges.size());
    myEdges = &edges;
  }
  
  /**Implementation of EffortCalculator **/
  double getEffort(_IntermodalEdge const *  edge ) const override
  {
    double  effort = 0;
    FareState const & state =  myFareStates.at( (size_t) edge->getNumericalID() );
    if( state.isValid() ){
      effort = state.myPriceDiff;
    }
    else{
      effort = std::numeric_limits<double>::max();
    }
    return effort;
  }
  
  /** Implementation of EffortCalculator **/
  void update(_IntermodalEdge const *  edge, _IntermodalEdge const *  prev)  override
  {
    
    std::string const & edgeType = edge->getLine();
    
    //get propagated fare state
    FareState  & state  = myFareStates.at( (size_t) prev->getNumericalID() );
  
    double oldPr;
    if( state.myFareToken == FareToken::START)
    {
      oldPr = 0;
    }
    else{
      oldPr = computePrice(state);
    }
    
    //treat  public transport edges
    if( edgeType.c_str()[0] != '!')
    {
      auto publicTransportEdge = static_cast<_PublicTransportEdge const *>(edge);
      updateFareState(state,*publicTransportEdge);
    }
    
    //treat stop edges
    else if( edgeType ==  "!stop" )
    {
      auto stopEdge = static_cast<_StopEdge const *>(edge);
      updateFareState(state,*stopEdge);
    }
    
    //treat ped edges
   else if( edgeType == "!ped")  {
      auto pedestrianEdge = static_cast<_PedestrianEdge const *>(edge);
      updateFareState(state,*pedestrianEdge);
    }
    
   else if( edgeType == "!access" ) {
      
      auto accessEdge = static_cast<_AccessEdge const *>(edge);
      updateFareState(state,*accessEdge, *prev);
    }
    
    else
    {
      updateFareState(state,*edge);
    }
    FareState & stateAtE = myFareStates[edge->getNumericalID()];
    double newPr = computePrice(stateAtE);
    stateAtE.myPriceDiff = newPr-oldPr;
    
    assert( stateAtE.myPriceDiff  >= 0 );
    
  }
  
  /** Implementation of EffortCalculator
   *  _IntermodalEdge should be an Connector Edge  **/
  void setInitialState(_IntermodalEdge const * edge) override
  {
    assert( edge->getLine() == "!connector");
    
    int id = edge->getNumericalID();
    
    myFareStates[id] = FareState(FareToken::START);
    
  }


private:
  /** List of all fare states **/
  std::vector<FareState> myFareStates;
  
  /** List of all edges **/
  std::vector<_IntermodalEdge*> const * myEdges;
  
  /** List of the prices **/
  Prices prices;
  
      double computePrice(FareState const & fareState) const
      {
        switch(fareState.myFareToken)
        {
          case FareToken ::H:
            return prices.halle;
          case FareToken ::L:
            return prices.leipzig;
          case FareToken ::T1:
            return prices.t1;
          case FareToken ::T2:
            return prices.t2;
          case FareToken ::T3:
            return prices.t3;
          case FareToken::U:
            return prices.zonePrices[0];
          case FareToken ::Z:
            return prices.zonePrices[fareState.myCounter.numZones() - 1];
          case FareToken ::M:
            return prices.maxPrice;
          case FareToken ::K:
            return prices.shortTrip;
          case FareToken ::KL:
          case FareToken ::KLZ:
          case FareToken ::KLU:
            return prices.shortTripLeipzig;
          case FareToken ::KH:
          case FareToken ::KHU:
          case FareToken ::KHZ:
            return prices.shortTripHalle;
          case FareToken::Free:
            return 1.4;
          case FareToken ::START:
            return 0;
          case FareToken::ZU:
          case FareToken::None:
            assert(false);
            
        }
        return std::numeric_limits<double>::max();
      }

      
      
  std::string output(_IntermodalEdge const * edge ) const override
  {
    
    FareState const  & my = myFareStates[edge->getNumericalID()];
    std::stringstream msg;
    if( false )
    {
      msg<<"Final fare state at edge of type: "<<edge->getLine()<<std::endl;
      msg<<"Faretoken"<<FareUtil::tokenToString(my.myFareToken)<<std::endl;
      msg<<"Price:"<<computePrice(my)<<std::endl;
      msg<<"Zones "<<my.myCounter.numZones()<<std::endl;
      msg<<"Stations: "<<my.myVisistedStops<<std::endl;
      msg<<"Distance:"<<my.myTravelledDistance<<std::endl;
    }
    else
    {
      msg << FareUtil::tokenToTicket(my.myFareToken)<<" ";
      if (my.myFareToken == FareToken::Z)
      {
        msg << my.myCounter.numZones()<< " ";
        if( my.myCounter.numZones() == 1 )
          msg << "Zone";
        else
          msg <<"Zonen";
        
      }
      else if( my.myFareToken == FareToken::U )
      {
        msg << my.myCounter.numZones() << "1 Zone";
  
      }
      msg<<":"<<computePrice(my);
    }
    return msg.str();
  }
  
  
  
  /** Collects faretoken at stopedge and determines new fare state **/
  inline void updateFareState( FareState const & currentFareState, _StopEdge const & e );

  /**updates the travelled distance **/
  inline void updateFareState( FareState const & currentFareState, _PedestrianEdge const & e );
  
  /**Destroys short-trip prices if it is the second time accessing public transport **/
  inline void updateFareState( FareState const & currentFareState, _AccessEdge const & e , _IntermodalEdge const & prev);
  
  /**Only propagates the fare state w/o any changes **/
  inline void updateFareState( FareState const & currentFareState, _IntermodalEdge const & e);
  
  /**Only propagates the fare state w/o any changes **/
  inline void updateFareState( FareState const & currentFareState, _PublicTransportEdge const & e );
  
};

template<class E, class L, class N, class V>
void FareModul<E,L,N,V>::updateFareState( FareState const & currentFareState, _StopEdge const & e )
{
  
  FareToken  collectedToken = e.getFareToken();
  
  //if station has no fare information, just propagate
  if( collectedToken  == FareToken::None ){
    std::cout<<"Progagating fare state for stop w/o a price!"<<std::endl;
    return;
  }
  
  FareToken const & token = currentFareState.myFareToken;
  
  FareState & stateAtE = myFareStates[e.getNumericalID()];
  
  stateAtE = currentFareState;
  
  stateAtE.myCounter.addZone( e.getFareZone() );
  
  stateAtE.myVisistedStops++;
  
  switch (token)
  {
    case FareToken ::Free:
      stateAtE.myFareToken = e.getStartToken();
      break;
    case FareToken::M :
      break;

    case FareToken::Z :
      if( stateAtE.myCounter.numZones() > 6 )
        stateAtE.myFareToken = FareToken::M;
      break;

    case FareToken::T1 :
    case FareToken::T2 :
    case FareToken::T3 :
      if( collectedToken == FareToken::Z )
        stateAtE.myFareToken = stateAtE.myTravelledDistance<=4000 ? FareToken::K : FareToken::Z;
      break;
    case FareToken::U :
      if( collectedToken == FareToken::H)
        stateAtE.myFareToken = FareToken::H;
      if( collectedToken == FareToken::L)
        stateAtE.myFareToken = FareToken::L;
      if( collectedToken == FareToken::Z)
        stateAtE.myFareToken = FareToken::Z;
      break;
    case FareToken::H:
    case FareToken::L:
      if(collectedToken ==FareToken::Z)
        stateAtE.myFareToken = FareToken::Z;
      break;
    case FareToken::KH:
      if( stateAtE.myVisistedStops <= 4 ){
        if( collectedToken == FareToken::U)
          stateAtE.myFareToken = FareToken::KHU;
        if( collectedToken == FareToken::Z)
          stateAtE.myFareToken = FareToken::KHZ;
      }
      else
      {
        if( collectedToken == FareToken::H )
          stateAtE.myFareToken=FareToken ::H;
        if (collectedToken == FareToken::Z)
          stateAtE.myFareToken = FareToken ::Z;
        if (collectedToken == FareToken::U)
          stateAtE.myFareToken = FareToken ::U;
      }
      break;
    case FareToken::KL:
      if( stateAtE.myVisistedStops <= 4 ){
        if( collectedToken == FareToken::U)
          stateAtE.myFareToken = FareToken::KLU;
        if( collectedToken == FareToken::Z )
          stateAtE.myFareToken = FareToken::KLZ;
      }
      else
      {
        if( collectedToken == FareToken::L )
          stateAtE.myFareToken=FareToken ::L;
        if (collectedToken == FareToken::Z)
          stateAtE.myFareToken = FareToken ::Z;
        if (collectedToken == FareToken::U)
          stateAtE.myFareToken = FareToken ::U;
      }
      break;
    case FareToken::K:
      if( stateAtE.myTravelledDistance > 4000  )
      {
        if( collectedToken == FareToken::U )
          stateAtE.myFareToken = FareToken ::U;
        if( collectedToken == FareToken::Z)
        {
          stateAtE.myFareToken = FareToken ::Z;
        }
      }
      break;
    case FareToken::KHU :
    case FareToken::KLU :
      if(stateAtE.myVisistedStops > 4 )
      {
        if( collectedToken == FareToken::U )
        stateAtE.myFareToken = FareToken::U;
      }
      break;
    
    case FareToken::KLZ:
    case FareToken::KHZ:
      if(stateAtE.myVisistedStops > 4 )
      {
        if( collectedToken == FareToken::Z )
          stateAtE.myFareToken = FareToken::Z;
      }
      break;
    case FareToken::ZU :
      assert(false);
      if( collectedToken == FareToken::U ){
        stateAtE.myFareToken=FareToken::U;
      }
      else {
        stateAtE.myFareToken=FareToken::Z;
      }

      break;
    case FareToken::None:
      std::cout<<"Reached invalid position in fareToken selection!"<<std::endl;
      assert(false);
      break;
  }
}

template<class E, class L, class N, class V>
void FareModul<E,L,N,V>::updateFareState(FareState const & currentFareState, _PedestrianEdge const & e)
{
  
  //only propagates the fare state
  FareState & stateAtE = myFareStates[e.getNumericalID()];
  
  stateAtE = currentFareState;
  
  if( currentFareState.myFareToken == FareToken::START )
  {
    stateAtE.myFareToken=FareToken::Free;
  }
  
}


template<class E, class L, class N, class V>
void FareModul<E,L,N,V>::updateFareState(FareState const & currentFareState, _PublicTransportEdge const & e ) {
  
  
  if( currentFareState.myFareToken == FareToken::None )
    return;
  
  FareState & stateAtE = myFareStates[e.getNumericalID()];
  
  stateAtE = currentFareState;
  stateAtE.myTravelledDistance += e.getLength();
}

template<class E, class L, class N, class V>
void FareModul<E,L,N,V>::updateFareState(FareState const & currentFareState, _IntermodalEdge const & e ) {
  
  if( currentFareState.myFareToken == FareToken::None )
    return;

  FareState & stateAtE = myFareStates[e.getNumericalID()];

  stateAtE = currentFareState;
  
  if( currentFareState.myFareToken == FareToken::START )
  {
    stateAtE.myFareToken=FareToken::Free;
  }

}

template<class E, class L, class N, class V>
void FareModul<E,L,N,V>::updateFareState(FareState const & currentFareState, const _AccessEdge &e, const _IntermodalEdge & prev) {
  
  FareToken const & token = currentFareState.myFareToken;
  
  FareState & stateAtE = myFareStates[e.getNumericalID()];
  
  stateAtE = currentFareState;
  
  if( currentFareState.myFareToken == FareToken::START )
  {
    stateAtE.myFareToken=FareToken::Free;
  }
  
  if( prev.getLine() == "!ped" )
  {
    switch (token){
    
      case FareToken::Free ://we have not yet taken public transport
        break;
      case  FareToken::K :
            if( currentFareState.myCounter.numZones() == 0 )
              stateAtE.myFareToken = FareToken::U;
            else
              stateAtE.myFareToken = FareToken::Z;
        break;
      case  FareToken::KH :
            stateAtE.myFareToken = FareToken::H;
        break;
      case  FareToken::KL :
            stateAtE.myFareToken = FareToken::L;
        break;
      case  FareToken::KLU :
           stateAtE.myFareToken = FareToken::L;
        break;
      case  FareToken::KHU:
           stateAtE.myFareToken = FareToken::H;
        break;
      case  FareToken::KLZ :
          stateAtE.myFareToken = FareToken::Z;
        break;
      case  FareToken::KHZ:
          stateAtE.myFareToken = FareToken::Z;
        break;
      default:
        return;
    }
  }

}



#endif //SUMO_FAREMODUL_H
