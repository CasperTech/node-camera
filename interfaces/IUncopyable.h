#pragma once

class IUncopyable
{
protected:
    IUncopyable()
    {
    }

    ~IUncopyable()
    {
    }

private:
    IUncopyable(const IUncopyable &) = delete;

    IUncopyable &operator=(const IUncopyable &) = delete;
};