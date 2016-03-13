# include <Siv3D.hpp>

void Main()
{
	Window::SetTitle(L"SkyRing");
	Graphics::SetBackground(Color(10, 120, 220));
	const Font font(14, Typeface::Heavy);
	const Sound soundMusic(L"Asset/bgm.wav", SoundLoop::All);
	const Sound soundWind(L"Asset/wind.mp3");
	const Sound soundRing(L"Asset/ring.mp3");
	const Sound soundFail(L"Asset/fail.mp3");
	const Mesh meshTerrain(MeshData::Plane(2000, { 20, 20 }));
	const Texture textureTerrain(L"Asset/Grass.jpg", TextureDesc::For3D);
	const Texture cloudTexture(L"Asset/c.png", TextureDesc::For3D);
	const VertexShader vsCloud(L"Asset/cloud.vs");
	const PixelShader psCloud(L"Asset/cloud.ps");
	if (!vsCloud || !psCloud)
	{
		return;
	}

	Array<Particle> clouds;
	for (int32 i = 0; i < 300; ++i)
	{
		const Vec3 center = RandomVec3({ -850.0, 850.0 }, { 180.0, 230.0 }, { -1000.0, 1000.0 });

		for (int32 n = 0; n < 8; ++n)
		{
			const Vec3 pos = center + RandomVec3() * Vec3(80, 20, 80);
			const Vec2 scale = RandomVec2({ 110,140 }, { 70,100 });
			const ColorF uOffset_centerPos(1.0 / 3 * Random(0, 2), center.x, center.y, center.z);
			clouds.emplace_back(pos, scale, uOffset_centerPos);
		}
	}

	Camera camera;
	Vec2 acceleration(0.0, 0.0);
	Vec2 speed(0.0, 0.0);
	Vec2 pos(0, 270);
	Vec3 ringPos(-200, 260, 1000);
	double terrainOffset = 0.0;
	int32 score = 0, highScore = 0;

	soundMusic.play();
	soundWind.playMulti(0.6);
	Stopwatch stopwatchWind(true);

	while (System::Update())
	{
		if (stopwatchWind.ms() > soundWind.lengthSec() * 850.0)
		{
			stopwatchWind.restart();
			soundWind.playMulti(0.6);
		}

		ringPos.z -= 3;
		if (ringPos.z < -997)
		{
			if (ringPos.xy().distanceFrom(pos) < 33)
			{
				soundRing.play();
				++score;
			}
			else
			{
				soundFail.play();
				highScore = Max(highScore, std::exchange(score, 0));
			}

			ringPos.set(Random(-360, 360), Random(255, 270), 2000);
		}

		const Vec2 dir = Vec2(Input::KeyRight.pressed - Input::KeyLeft.pressed, Input::KeyDown.pressed - Input::KeyUp.pressed)* 0.0001;
		acceleration.set(Clamp(acceleration.x + dir.x, -0.01, 0.01), Clamp(acceleration.y + dir.y, -0.01, 0.01));
		speed.set(Clamp(speed.x + acceleration.x, -1.0, 1.0), Clamp(speed.y + acceleration.y, -1.0, 1.0));
		pos.set(Clamp(pos.x + speed.x, -480., 480.), Clamp(pos.y + speed.y, 220., 280.));
		camera.pos.set(pos.x, pos.y, -1000);
		camera.lookat.set(camera.pos + Vec3(speed.x, speed.y * 2.0 - 2.0, 10));
		camera.up.set(speed.x * 0.1, 1.0, 0.0);
		Graphics3D::SetCamera(camera);

		terrainOffset -= 3.0;
		if (terrainOffset < -2000)
		{
			terrainOffset += 2000;
		}

		for (auto i : step(3))
		{
			meshTerrain.translated(0, 0, terrainOffset + i * 2000).draw(textureTerrain);
		}

		Graphics3D::SetDepthStateForward(DepthState::TestWrite);
		Graphics3D::SetAmbientLightForward(ColorF(0.8));
		for (auto i : step(6))
		{
			Box(-500, 80 + i * 80, 0, 20, 20, 20000).drawForward(ColorF(0.2, 1.0, 0.4, 0.4));
			Box( 500, 80 + i * 80, 0, 20, 20, 20000).drawForward(ColorF(0.2, 1.0, 0.4, 0.4));
		}

		for (auto i : step(12))
		{
			Box(8).asMesh().rotated(0.0, 0.0, i * -30_deg).translated(ringPos + Spherical(30, i * 30_deg, 0)).drawForward(ColorF(1.0, 0.3, 0.0, 0.9));
		}

		Graphics3D::SetDepthStateForward(DepthState::TestOnly);
		Graphics3D::BeginVSForward(vsCloud);
		Graphics3D::BeginPSForward(psCloud);
		Graphics3D::DrawParticlesForward(clouds, cloudTexture);
		Graphics3D::EndVSForward();
		Graphics3D::EndPSForward();

		for (auto& cloud : clouds)
		{
			cloud.pos.z -= 3;
			cloud.color.w -= 3;

			if (cloud.pos.z < -1000)
			{
				cloud.pos.z += 2000;
				cloud.color.w += 2000;
			}
		}
		std::sort(clouds.begin(), clouds.end(), [](const auto& a, const auto& b){ return a.pos.z > b.pos.z; });

		font(L"{} (ハイスコア:{})"_fmt, score, highScore).draw(10, 450, Color(255, 120));
	}
}
