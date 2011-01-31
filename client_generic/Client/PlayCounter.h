/*
 *  PlayCounter.h
 *  ElectricSheep
 *
 *  Created by Daniel Svoboda on 4/25/09.
 *
 */

#ifndef _PLAYCOUNTER_H
#define _PLAYCOUNTER_H

#include	<boost/filesystem/path.hpp>
#include	<boost/scoped_ptr.hpp>
static const int gl_sMaxGeneration = 100000;
#define max_sheep 100000
#define max_play_count ((1<<16)-1)
#define log_page_size 10
#define log_count_size (log_page_size-1)
#define play_count_size (1<<log_count_size)
#define n_dirty_bits (1+(max_sheep>>log_count_size))
#define play_write_rate 10

using boost::filesystem::path;

struct sPlayCountData
{
	uint16	PlayCounts[ max_sheep ];
	FILE*	PlayCountFile;
};

class	CPlayCounter : public Base::CSingleton<CPlayCounter>
{
	friend class Base::CSingleton<CPlayCounter>;

	std::map<int, sPlayCountData> m_PlayCounts;
	path m_PlayCountFilePath;
	bool m_ReadOnly;

	size_t	m_DeadEndCutSurvivors;
	size_t	m_MedianCutSurvivors;

	int	m_PlayCountDecayY;
	int	m_PlayCountDecayZ;

	uint64	m_PlayCountTotal;

	void InitPlayCounts(int generation)
	{
		if (generation >= gl_sMaxGeneration || generation == 0)
			return;
		m_PlayCountTotal = 0;

		std::map<int, sPlayCountData>::iterator iter = m_PlayCounts.find(generation);
		if (iter == m_PlayCounts.end())
		{
			boost::scoped_ptr<sPlayCountData> pdata(new sPlayCountData);
			memset(pdata.get(), 0, sizeof(sPlayCountData));
			path generation_path = m_PlayCountFilePath;
			std::stringstream generationstr;
			generationstr << "play_counts." << generation;
			generation_path /= generationstr.str().c_str();
			// create new file for this generation
#ifdef WIN32
			pdata->PlayCountFile = _fsopen(generation_path.native_file_string().c_str(), "r+b", _SH_DENYWR); // deny write
#else
			pdata->PlayCountFile = fopen(generation_path.native_file_string().c_str(), "r+b"); // deny write
#endif
			if (pdata->PlayCountFile == NULL) // file not found
			{
#ifdef WIN32
				pdata->PlayCountFile = _fsopen(generation_path.native_file_string().c_str(), "w+b", _SH_DENYWR);
#else
				pdata->PlayCountFile = fopen(generation_path.native_file_string().c_str(), "w+b");
#endif
				if ( pdata->PlayCountFile == NULL ) // unable to open for writing
				{
					pdata->PlayCountFile = fopen(generation_path.native_file_string().c_str(), "rb" );
					if ( pdata->PlayCountFile == NULL ) // unable to open for reading
					{
						m_ReadOnly = true;
						g_Log->Error( "Running without play counts: %s",  generation_path.native_file_string().c_str());
						m_PlayCounts[generation] = *pdata;
						return;
					}
					else
					{
						g_Log->Warning( "Using read-only playcounts" );
						m_ReadOnly = true;
					}
				}
				else // write after w+b
				if ( max_sheep != fwrite( pdata->PlayCounts, sizeof( uint16 ), max_sheep, pdata->PlayCountFile ) )
				{
					m_ReadOnly = true;
					g_Log->Error( "Cannot initialize play counts, fwrite failed" );
					fclose( pdata->PlayCountFile );
					pdata->PlayCountFile = NULL;
					m_PlayCounts[generation] = *pdata;
					return;
				}
				else
					fflush(pdata->PlayCountFile);
			}


			size_t nread = fread( pdata->PlayCounts, sizeof( uint16 ), max_sheep, pdata->PlayCountFile);
			fflush(pdata->PlayCountFile);
			if ( nread < max_sheep )
			{
				g_Log->Error( "Cannot initialize play counts, fread failed" );
				fclose( pdata->PlayCountFile );
				pdata->PlayCountFile = NULL;
			}
			
			m_PlayCounts[generation] = *pdata;
			
			if ( nread == max_sheep )
				for (size_t ii = 0; ii < max_sheep; ++ii)
					m_PlayCountTotal += m_PlayCounts[generation].PlayCounts[ii];
		}
	}

public:
	CPlayCounter():m_ReadOnly(false), m_DeadEndCutSurvivors(0), m_MedianCutSurvivors(0), m_PlayCountTotal(0)
	{
		m_PlayCountDecayY = g_Settings()->Get( "settings.player.PlayCountDecayY", 2000 );
		m_PlayCountDecayZ = g_Settings()->Get( "settings.player.PlayCountDecayZ", 60 );
		if (m_PlayCountDecayZ < 0)
			m_PlayCountDecayZ = 0;
		if (m_PlayCountDecayZ > 100)
			m_PlayCountDecayZ = 100;
	}
	
	virtual ~CPlayCounter()
	{
		Shutdown();
		
		SingletonActive( false );
	}
	
	const char *Description()	{	return( "Player" );	};
	
	void clearMedianSurvivorsStats()
	{
		m_MedianCutSurvivors = 0;
	}

	void clearDeadEndSurvivorsStats()
	{
		m_DeadEndCutSurvivors = 0;
	}

	void IncMedianCutSurvivors()
	{
		++m_MedianCutSurvivors;
	}

	void IncDeadEndCutSurvivors()
	{
		++m_DeadEndCutSurvivors;
	}

	uint64 GetMedianCutSurvivors()
	{
		return m_MedianCutSurvivors;
	}

	uint64 GetDeadEndCutSurvivors()
	{
		return m_DeadEndCutSurvivors;
	}

	void ClosePlayCounts()
	{
		std::map<int, sPlayCountData>::iterator iter = m_PlayCounts.begin();
		while (iter != m_PlayCounts.end())
		{
			if (iter->second.PlayCountFile != NULL)
			{
				fflush( iter->second.PlayCountFile );
				fseek( iter->second.PlayCountFile, 0, SEEK_SET );
				int nwrite = fwrite( iter->second.PlayCounts, sizeof( uint16 ), max_sheep, iter->second.PlayCountFile );
				if (nwrite != max_sheep)
					g_Log->Error( "Writing playcounts failed" );
				fclose( iter->second.PlayCountFile );
				iter->second.PlayCountFile = NULL;
			}
			++iter;
		}
		m_PlayCounts.clear();
	}

	const bool Shutdown( void )
	{ 
		ClosePlayCounts();

		return true;
	}
	
	void SetDirectory( const path& dir )
	{
		m_PlayCountFilePath = dir;
	}

	void IncPlayCount( int generation, int id )
	{
		if (id >= max_sheep || generation > gl_sMaxGeneration || generation == 0)
			return;
		if (m_PlayCountTotal > m_PlayCountDecayY && m_PlayCountDecayZ != 100)
		{
			m_PlayCountTotal = 0;
			for (std::map<int, sPlayCountData>::iterator jj = m_PlayCounts.begin(); jj != m_PlayCounts.end(); ++jj)
			for (size_t ii = 0; ii < max_sheep; ++ii)
			{
				jj->second.PlayCounts[ii] = uint16(m_PlayCountDecayZ/100. * jj->second.PlayCounts[ii]);
				m_PlayCountTotal += jj->second.PlayCounts[ii];
			}
		}

		std::map<int, sPlayCountData>::iterator iter = m_PlayCounts.find(generation);
		if (iter == m_PlayCounts.end())
			InitPlayCounts(generation);

		iter = m_PlayCounts.find(generation);
		++iter->second.PlayCounts[id];
		++m_PlayCountTotal;
	}

	uint16 PlayCount( int generation, int id )
	{
		if ( id < max_sheep  && generation != 0 && generation < gl_sMaxGeneration)
		{
			std::map<int, sPlayCountData>::iterator iter = m_PlayCounts.find(generation);
			if (iter == m_PlayCounts.end())
				InitPlayCounts(generation);

			iter = m_PlayCounts.find(generation);
			if (iter->second.PlayCounts[id] != 0)
				return iter->second.PlayCounts[id];
			else
				return 1;
		}
		return 1;
	}

	bool ReadOnlyPlayCounts()
	{
		return m_ReadOnly;
	}
};

/*
	Helper for less typing...

*/
inline CPlayCounter &g_PlayCounter( void )	{ return( CPlayCounter::Instance() ); }

#endif //_PLAYCOUNTER_H

