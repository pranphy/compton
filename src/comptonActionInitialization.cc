/*
 * comptonUserActionInitialization.cc
 *
 *  Created on: May 1, 2017
 *      Author: wdconinc
 */

#include "comptonActionInitialization.hh"

#include "comptonRunAction.hh"
#include "comptonEventAction.hh"
#include "comptonTrackingAction.hh"
#include "comptonSteppingAction.hh"
#include "comptonPrimaryGeneratorAction.hh"

void comptonActionInitialization::Build() const
{
  // Run action
  comptonRunAction* run_action = new comptonRunAction();
  SetUserAction(run_action);

  // Event action
  comptonEventAction* event_action = new comptonEventAction();
  SetUserAction(event_action);

  // Tracking action
  comptonTrackingAction* tracking_action = new comptonTrackingAction();
  SetUserAction(tracking_action);

  // Stepping action
  comptonSteppingAction* stepping_action = new comptonSteppingAction();
  SetUserAction(stepping_action);

  // Primary generator action
  comptonPrimaryGeneratorAction* gen_action = new comptonPrimaryGeneratorAction();
  SetUserAction(gen_action);
  event_action->SetPrimaryGeneratorAction(gen_action);
}

void comptonActionInitialization::BuildForMaster() const
{
  // Run action
  comptonRunAction* run_action = new comptonRunAction();
  SetUserAction(run_action);
}
