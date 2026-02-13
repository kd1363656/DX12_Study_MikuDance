#pragma once
#include <memory>

class Dx12Wrapper;

class Render
{
public:

	Render (std::shared_ptr<Dx12Wrapper>& dx);
	Render() = default;
	~Render() = default;

	void Frame();

private:

	void Update() const;
	void DrawFrame() const;
	void EndOfFrame() const;
};