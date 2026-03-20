# Compton Generator

Look at [Compton Scattering Notes](compton-scattering.md) for the mathematical calculation of compton scattering kinematics.
# Code implementation

The generator produces a scattered photon and electron in the **lab frame**, even
when the incoming electron beam has a small angular offset. This is implemented
in `comptonGenCompton.cc`.

The workflow is:

 1. Sample the initial electron direction.
 2. Rotate the coordinate system so the electron aligns with the \f$+z\f$ axis.
 3. Perform Compton scattering in this rotated frame (the “ez” frame).
 4. Rotate the scattered momenta back to the lab frame.


## Sampling the initial electron direction

The electron beam has a small angular spread. We sample its direction uniformly in \f$\cos\theta\f$:

\begin{align*}
    \cos\theta_0 \in [\cos\theta_{\min},\cos\theta_{\max} ]
\end{align*}

This ensures isotropic sampling over a cone.

```cpp
double theta0 = std::acos(CLHEP::RandFlat::shoot(std::cos(fInitialAngleMin), std::cos(fInitialAngleMax)));
double phi0   = 0.0;
```

The azimuth \f$\phi_0\f$ is set to zero because the divergence is assumed to lie in
a single plane.


## Rotate to align the electron with \f$+z\f$

We construct a rotation that maps the actual electron direction to the \f$+z\f$ axis:

\begin{align*}
    R_{\text{to ez}}\,\hat{p}_e^{\text{lab}} = \hat{z}.
\end{align*}

```cpp
G4RotationMatrix R_to_ez;
R_to_ez.rotateZ(-phi0);
R_to_ez.rotateY(-theta0);

G4RotationMatrix R_to_lab = R_to_ez.inverse();
```

All scattering is performed in this rotated frame. After scattering, we rotate back:

\begin{align*}
    \vec{p}^{\text{lab}} = R_{\text{to lab}}\,\vec{p}^{\text{ez}}.
\end{align*}


## Sampling the scattered photon energy

A random Compton energy fraction \f$\rho\f$ is drawn:

\begin{align*}
k' = \rho\,k'_{\max}.
\end{align*}

The maximum scattered photon energy is:

\begin{align*}
k'_{\max} = \frac{4\gamma^2 k}{1 + 4\gamma k/m_e}.
\end{align*}

```cpp
double gamma_E = rho * fMaxPhotonEnergy;
G4double gma = fElectronEnergy / CLHEP::electron_mass_c2;
```


## Photon scattering angle in the electron‑aligned frame

The code uses the analytic inverse‑Compton relation:

\begin{align*}
\theta_\gamma = \sqrt{ \frac{4k}{k'} - \frac{1}{a\gamma^2} },
\end{align*}

where

\begin{align*}
a = \frac{1}{1 + 4\gamma k/m_e}.
\end{align*}

```cpp
double gamma_theta = std::sqrt( 4.0 * fLaserEnergy/gamma_E
                                - 1.0/(fAParameter * gma * gma) );
double gamma_phi = CLHEP::RandFlat::shoot(2.0 * CLHEP::pi);
```

The photon direction in the electron‑aligned frame is then:

\begin{align*}
    \hat{k}'_{\text{ez}} = (\theta_\gamma,\phi_\gamma).
\end{align*}

```cpp
G4ThreeVector gamma_dir_ez;
gamma_dir_ez.setRThetaPhi(1.0, gamma_theta/CLHEP::radian, gamma_phi/CLHEP::radian);
```


## Rotating the photon back to the lab frame

The scattered photon momentum is rotated back:

\begin{align*}
\vec{k}'_{\text{lab}} = R_{\text{to lab}}\,\vec{k}'_{\text{ez}}.
\end{align*}

```cpp
G4ThreeVector gamma_momentum_ez = gamma_E * gamma_dir_ez;
G4ThreeVector gamma_momentum_lab = R_to_lab * gamma_momentum_ez;

evt->ProduceNewParticle(G4ThreeVector(0,0,0), gamma_momentum_lab, "gamma");
```


## Scattered electron energy and angle

Energy conservation gives:

\begin{align*}
E_e' = E_e + k - k'.
\end{align*}

```cpp
double electronE = (fElectronEnergy + fLaserEnergy) - gamma_E;
```

Electron momentum magnitude:

\begin{align*}
p_e' = \sqrt{E_e'^2 - m_e^2}.
\end{align*}

```cpp
double momentum_e = std::sqrt(electronE*electronE
                              - CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
```

The electron scattering angle follows from **transverse momentum conservation**:

\begin{align*}
p_e' \sin\theta_e = k' \sin\theta_\gamma.
\end{align*}

Thus:

\begin{align*}
    \theta_e = \sin^{-1}\left( \frac{k'}{p_e'} \sin\theta_\gamma \right).
\end{align*}

```cpp
double theta_e = std::asin(gamma_E / momentum_e * std::sin(gamma_theta));
double phi_e   = -gamma_phi;
```

The electron azimuth is opposite to the photon’s because the scattering is
planar:

\begin{align*}
\phi_e = -\phi_\gamma.
\end{align*}


## Rotating the electron back to the lab frame

The electron momentum is rotated back:

\begin{align*}
\vec{p}_e'^{\,\text{lab}} = R_{\text{to lab}}\,\vec{p}_e'^{\,\text{ez}}.
\end{align*}

```cpp
G4ThreeVector e_dir_ez;
e_dir_ez.setRThetaPhi(1.0, theta_e/CLHEP::radian, phi_e/CLHEP::radian);

G4ThreeVector e_momentum_ez = momentum_e * e_dir_ez;
G4ThreeVector e_momentum_lab = R_to_lab * e_momentum_ez;

evt->ProduceNewParticle(G4ThreeVector(0,0,0), e_momentum_lab, "e-");
```


## Summary of the physics workflow

 1. Sample initial electron direction \f$(\theta_0,\phi_0)\f$.
 2. Rotate coordinates so the electron aligns with \f$+z\f$.
 3. Sample Compton energy fraction \f$\rho\f$.
 4. Compute photon scattering angle \f$\theta_\gamma\f$ from  kinematics.
 5. Compute electron scattering angle \f$\theta_e\f$ from momentum conservation.
 6. Rotate both scattered particles back to the lab frame.
 7. Produce the photon and electron in the event.

# Macro commands

The compton generator takes the following macro

```mac
/compton/evgen/compton/beamene 11.0 GeV
/compton/evgen/compton/beamcurr 65.0 microampere
/compton/evgen/compton/laserwavelength 532.0 nm
/compton/evgen/compton/crossingangle 1.15 deg
/compton/evgen/compton/laserpower 2000.0 watt
/compton/evgen/compton/ethmin 0.100 milliradian
/compton/evgen/compton/ethmax 0.200 milliradian
/compton/evgen/compton/sigmae 60.0 um
/compton/evgen/compton/sigmap 45.0 um
```

The parameter names are self explanatory.
