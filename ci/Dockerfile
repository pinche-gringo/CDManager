# Base image for the CI-jobs of CDManager (see .gitlab-ci.yml)
#
# Changing this file rebuilds the image on the next push
FROM fedora:45

# The version of libYGP/libXGP; its RPMs are taken from the GitHub release
ARG YGP_VERSION=0.9.70
ARG YGP_URL=https://github.com/pinche-gringo/libYGP/releases/download/libYGP_V${YGP_VERSION}

RUN dnf install -y --setopt=install_weak_deps=False \
        gcc gcc-c++ meson ninja-build git gettext gettext-devel rpm-build \
        boost-devel gtkmm4.0-devel libsoup3-devel libpq-devel \
        curl \
        ${YGP_URL}/libYGP-${YGP_VERSION}-1.fc45.x86_64.rpm \
        ${YGP_URL}/libYGP-devel-${YGP_VERSION}-1.fc45.x86_64.rpm \
        ${YGP_URL}/libXGP-${YGP_VERSION}-1.fc45.x86_64.rpm \
        ${YGP_URL}/libXGP-devel-${YGP_VERSION}-1.fc45.x86_64.rpm \
    && dnf clean all
